%##########################################################################
% Copyright (c) 2011 - 2018 Qualcomm Technologies International, Ltd

classdef kcd < handle
  % KCD Wrapper class for core_debug_kalimba.dll.

%=Constant=properties====================================================
properties (Constant)
    verbose = false; % true;
    VAR_INFO_ELEMENTS = 8;
end

%=Properties=============================================================
properties (SetAccess=private)
    cd;
    ka = kxs.instance();
    connectString;
    coreUri;
    isConnected = false;
    symFiles = {};
    symFileIndex = 1;
    symbols = {};
    symbolIndex = 1;
    globals = {};
    types;
    device;
    dm_index;
    pm_index;

    noConnectionMessage = 'No connection state, please connect..';
    writeLengthMismatchMessage = 'must have the same number of values as addresses';
    accessNoWordsMessage = 'Cannot access zero words of memory from the DSP';
    end

properties (SetAccess=public)
    isLoaded;
end

%=Public=methods=========================================================
methods(Access=public)

    %-Private-singleton-constructor:-must-go-via-instance()----------------
    function newObj = kcd()
      newObj.init();
    end
end

%=Private=methods========================================================
methods(Access=private)

    %-Disconnect-any-existing-core-debug-session---------------------------
    function pre_connect(obj)
        if obj.cd ~= 0
            obj.disconnect();
        end
        if ~libisloaded('core_debug_kalimba')
            kcd.load();
        end
    end

    %-(Re-)Initialise-the-object-data--------------------------------------
    function init(obj)
        obj.isConnected = false;
        obj.connectString = '';
        obj.cd = 0;
        obj.symFiles = {};
        obj.symFileIndex = 1;
        obj.symbols = {};
        obj.symbolIndex = 1;
        obj.globals = {};
    end

    %-Add-a-symbol-to-the-simple-symbol-table------------------------------
    function add_to_symbols(obj,varinfo)
        if ~ischar(varinfo{(3)})
            % name
            obj.symbols(obj.symbolIndex,1) = varinfo(1);
            % size
            obj.symbols(obj.symbolIndex,2) = varinfo(2);
            % address
            obj.symbols(obj.symbolIndex,3) = varinfo(3);
            obj.symbolIndex = obj.symbolIndex + 1;
        end
    end

    %-Retrieve-list-of-all-known-types-------------------------------------
    function get_types(obj)
        typelist_struct=libstruct('cd_types_list',{});
        err = calllib('core_debug_kalimba','cd_get_types',obj.cd,typelist_struct);
        kcd.check_error(err);
        typelist = calllib('core_debug_kalimba','cd_unpack_types_list',typelist_struct);
        kcd.check_error(err);
        % For each known type
        typecount = typelist_struct.number_of_types;
        obj.types = cell(typecount, 3);
        for i=1: typecount
            cd_type = calllib('core_debug_kalimba','cd_access_type_element',typelist,i-1);
            kcd.check_error(err);
            obj.types(i,:) = obj.get_typeinfo(cd_type);
        end

        err = calllib('core_debug_kalimba','cd_free_types',typelist_struct);
        kcd.check_error(err);
    end

    %-Retrieve-information-for-a-single-type-------------------------------
    function typeinfo = get_typeinfo(~, cd_type)
        s = libpointer('stringPtrPtr',{''});
        [err,~,type_name] = calllib('core_debug_kalimba','cd_get_type_name',cd_type,s);
        kcd.check_error(err, true);
        [err,~,type_kind] = calllib('core_debug_kalimba','cd_get_kind_of_type',cd_type,0);
        if strcmp(err,'CD_ERR_NO_ERROR')
%             if strcmp('CD_ARRAY_TYPE',type_kind)
%                 [err,~,array_elements] = calllib('core_debug_kalimba','cd_get_array_type_size',cd_type,0);
%                 kcd.check_error(err, true);
%             end
            [err,~,type_size] = calllib('core_debug_kalimba','cd_get_type_size',cd_type,0);
            kcd.check_error(err, true);
            size = double(type_size);
            if strcmp(err,'CD_ERR_NO_ERROR')
                typeinfo(1, :) = {type_name{1}, size, type_kind};
            else
                typeinfo(1, :) = {type_name{1}, err, type_kind};
            end
        else
            typeinfo(1, :) = {type_name{1}, err, err};
        end
    end

    %-Retrieve-list-of-all-signatures--------------------------------------
    function get_signatures(obj)
        siglist_struct = libstruct('s_cd_signature_list',{});
        err = calllib('core_debug_kalimba','cd_get_session_signatures', obj.cd, siglist_struct);
        kcd.check_error(err, true);
        siglist = calllib('core_debug_kalimba','cd_unpack_session_signatures_list',siglist_struct);
        % For each signature
        sigcount = siglist_struct.number_of_signatures;
        obj.sigs = cell(sigcount);
        for i=1: sigcount
            sig = calllib('core_debug_kalimba','cd_access_session_signature_element',siglist,i-1);
            kcd.check_error(err);
            obj.sigs(i) = sig;
        end

        err = calllib('core_debug_kalimba','cd_free_session_signatures',siglist_struct);
        kcd.check_error(err, true);
    end

    %-Retrieve-information-from-a-list-of-variables------------------------
    function varlistinfo = read_varlist(obj,varlist)
        index = 1;
        varcount = varlist.number_of_variables;
        varlistinfo = cell(varcount, obj.VAR_INFO_ELEMENTS);
        if ~varcount
            return
        end
        vars = calllib('core_debug_kalimba', 'cd_unpack_variables_list', varlist);
        for i = 1: varcount
            % Get variable handle
            sym = calllib('core_debug_kalimba', 'cd_access_variable_element', vars, i-1);
            varlistinfo(index, :) = obj.get_varinfo(sym);
            index = index + 1;
        end
    end

    %-Retrieve-information-for-a-single-variable---------------------------
    function varinfo = get_varinfo(obj, var)
        varinfo = {};
        [err, ~, varname] = calllib('core_debug_kalimba','cd_get_variable_name', var, {''});
        kcd.check_error(err, true);
        [err, ~, varlife] = calllib('core_debug_kalimba','cd_get_variable_lifetime_class', var, libpointer('cd_variable_lifetime_classification', 0));
        kcd.check_error(err, true);
        cd_type=libstruct('s_cd_type');
        [err,~,~] = calllib('core_debug_kalimba','cd_get_type_of_variable',var,cd_type);
        kcd.check_error(err, true);
        typeinfo = obj.get_typeinfo(cd_type);
        type_name = typeinfo{1};
        type_size = typeinfo{2};
        type_kind = typeinfo{3};

        % get current address
        logaddr = libpointer('cd_logical_addressPtr');
        err = calllib('core_debug_kalimba', 'cd_get_address_of_variable', obj.cd, var, logaddr);
        if ~strcmp('CD_ERR_NO_ERROR',err)
            varaddr = err;
            address_space_index = err;
            varloc = {err};
        else
            kcd.check_error(err,true);
            [err, ~, varaddr, address_space_index] = ...
                calllib('core_debug_kalimba', 'cd_get_logical_address_value', logaddr, 0, 0, 0);
            varaddr = double(varaddr);
            kcd.check_error(err, true);
            err = calllib('core_debug_kalimba', 'cd_free_logical_address', logaddr);
            kcd.check_error(err, true);

            % get location info
            [err, ~, ~, varloc] = calllib('core_debug_kalimba', 'cd_get_variable_location_info', obj.cd, var, {''});
            if strcmp('CD_ERR_NO_LOADER_ENTRY',err)
                varloc = {err};
            else
                kcd.check_error(err, true);
            end
        end

        % populate return value
        varinfo(1, :) = { varname{1}, type_size, varaddr, ...
            type_kind, varloc{1}, varlife, type_name, address_space_index};
    end
end

%=Public=static=methods==================================================
methods(Static, Access=public)

    %-Retrieve-the-singleton-instance--------------------------------------
    function obj = instance()
        persistent uniqueInstance
        if isempty(uniqueInstance)
            obj = kcd();
            obj.load();
            uniqueInstance = obj;
        else
            obj = uniqueInstance;
        end
    end

    %-Unit-test------------------------------------------------------------
    function test()
        k = kcd.instance();
        k.load('device://tcp/localhost:31400/spi/sim/csra68100/audio/p0');
        k.isConnected();
        k.disconnect();
        k.unload();
    end

    %-Inspect-error-status-and-throw-an-error-if-required------------------
    function status = check_error(error_string, continue_on_error)
        no_error = libpointer('cd_err',0);
        if ~strcmp(no_error.value, error_string)
            [~, errStr] = calllib('core_debug_kalimba','cd_get_last_error',{''});
            if nargin < 2 || ~continue_on_error
                error(['core_debug_kalimba: ', error_string, ' - ', errStr{1}]);
            else
                warning(['core_debug_kalimba: ', error_string, ' - ', errStr{1}]);
            end
            status = true;
            return
        end
        status = false;
    end

    %-Load-the-DLL---------------------------------------------------------
    function load()

        % Re/load the DLL.
        if libisloaded('core_debug_kalimba')
            unloadlibrary('core_debug_kalimba');
            kcd.log('dll unload');
        end
        % suppress some expected warnings
        warning('off','MATLAB:loadlibrary:TypeNotFound')
        warning('off','MATLAB:loadlibrary:InvalidFunctionReturnType')
        loadlibrary('core_debug_kalimba', @core_debug_proto);
        warning('on','MATLAB:loadlibrary:TypeNotFound')
        warning('on','MATLAB:loadlibrary:InvalidFunctionReturnType')

        kcd.log('dll load');
        kcd.loaded = true;
    end
end

%=Properties=methods=====================================================
methods

    %-Is-there-an-active-session-with-the-DLL?-----------------------------
    function value = get.isConnected(obj)
        value = obj.cd ~= 0;
    end

end

%=Private=static=methods=================================================
methods(Static, Access=private)

    %-If-in-verbose-mode-then-log-messages-to-the-console------------------
    function log(str)
        if kcd.verbose
            fprintf('kcd %s\n', str);
        end
    end

end

%=Public=instance=methods================================================
methods(Access = public)

      %-Delete-the-register-names-and-unload-the-dll-----------------------
    function unload(obj)
        if obj.cd ~= 0
            obj.disconnect();
        end
        unloadlibrary('core_debug_kalimba');
        kcd.log('dll unload');
    end

    %-Connect-to-the-specified device--------------------------------------
    function connect(obj, coreUriString)
        assert(ischar(coreUriString))

        obj.pre_connect();

        if obj.isConnected
            obj.disconnect()
        end

        dll_handle = libstruct('s_coredbg');
        err = calllib('core_debug_kalimba','cd_open', dll_handle);
        kcd.check_error(err);

        % connect to a core URI
        [err, ~, obj.connectString] = ...
            calllib('core_debug_kalimba', ...
                'cd_connect', ...
                dll_handle, ...
                coreUriString);
        kcd.check_error(err);

        kcd.log(['cd connect via core URI ', coreUriString]);
        obj.cd = dll_handle;

        % Read program and data adddress space index values
        [err, ~, obj.dm_index] = calllib('core_debug_kalimba', ...
            'cd_get_data_address_space_index',obj.cd,99);
        kcd.check_error(err);
        [err, ~, obj.pm_index] = calllib('core_debug_kalimba', ...
            'cd_get_program_address_space_index',obj.cd,99);
        kcd.check_error(err);
    end

    %-Disconnect-an-active-session-----------------------------------------
    function disconnect(obj)
        assert(obj.cd ~= 0, 'No connection');
        obj.remove_symfiles();
        err = calllib('core_debug_kalimba','cd_disconnect',obj.cd);
        kcd.check_error(err);
        kcd.log('cd disconnect');
        err = calllib('core_debug_kalimba','cd_close', obj.cd);
        kcd.check_error(err);
        kcd.log('cd close');
        obj.init();
    end

    %-Load-a-symbol-file-in-core-debug-------------------------------------
    function add_symfile(obj, symfile)
        assert(obj.cd ~= 0, 'No connection');
        obj.symFiles{obj.symFileIndex} = libstruct('s_cd_symbolfile');
        err = calllib('core_debug_kalimba', 'cd_add_symbol_file', ...
            symfile, obj.cd, obj.symFiles{obj.symFileIndex});
        kcd.check_error(err);
        obj.symFileIndex = obj.symFileIndex + 1;
    end

    %-Unload-all-symbol-files-from-core-debug------------------------------
    function remove_symfiles(obj)
        sym_file_count = numel(obj.symFiles);
        if sym_file_count
            for i = 1: sym_file_count
                err = calllib('core_debug_kalimba', 'cd_remove_symbol_file', obj.symFiles{i});
                kcd.check_error(err);
            end
            obj.symFiles = {};
        end
    end

    %-Read-core-debug-symbol-data------------------------------------------
    function cd_symbols = get_symbols(obj)
        % obj.get_types(); % not currently needed
        % obj.get_signatures(); % not currently needed
        obj.symbolIndex = 1; % reset symbol index

        % Read device tree from core debug
        obj.device = obj.get_cu_info();

        % Read global data
        obj.globals = obj.get_globals();

        % Return the collected symbol list
        cd_symbols = obj.symbols;
    end

    %-Read-the-core-global-data-table--------------------------------------
    function globals = get_globals(obj)
        global_vars = libstruct('s_cd_variables_list',{});
        err = calllib('core_debug_kalimba','cd_get_globals', obj.cd, global_vars);
        kcd.check_error(err);
        globals = obj.read_varlist(global_vars);
        err = calllib('core_debug_kalimba', 'cd_free_variables', obj.cd, global_vars);
        kcd.check_error(err);
    end

    %-Load-symbol-information-from-compile-units---------------------------
    function symbols = get_cu_info(obj)
        func_index = 1;
        glob_index = 1;
        stat_index = 1;
        func_stat_index = 1;
        symbols = {};
        % For each symbol file loaded in device
        for i = 1: obj.symFileIndex-1

            % get the list of compile directories
            compile_dir_list_struct = libstruct('s_cd_compile_dir_list',{});
            err = calllib('core_debug_kalimba','cd_get_compile_dir_list', obj.symFiles{i}, compile_dir_list_struct);
            kcd.check_error(err);
            compile_dir_list = calllib('core_debug_kalimba', 'cd_unpack_compile_dir_list', ...
                compile_dir_list_struct);

            % for each compile directory in this symbol file
            compile_dirs = compile_dir_list_struct.number_of_compile_dirs;
            for j = 1: compile_dirs
                compile_dir = calllib('core_debug_kalimba', 'cd_access_compile_dir_element', compile_dir_list, j-1);
                [err, ~, path] = calllib('core_debug_kalimba','cd_get_compile_dir_path', compile_dir,{''});
                kcd.check_error(err);
                symbols(i).compile_dir(j).path = path;

                % get the list of compile units
                compile_unit_list_struct=libstruct('s_cd_compile_unit_list',{});
                err = calllib('core_debug_kalimba','cd_get_compile_units',compile_dir,compile_unit_list_struct);
                kcd.check_error(err);
                compile_unit_list = calllib('core_debug_kalimba', 'cd_unpack_compile_unit_list', ...
                    compile_unit_list_struct);

                % For each compilation unit in this compile directory
                compile_units = compile_unit_list_struct.num_of_compile_units;
                for k = 1: compile_units
                    compile_unit = calllib('core_debug_kalimba', 'cd_access_compile_unit_element', ...
                        compile_unit_list, k-1);
                    [err, ~, cu_log_path] = calllib('core_debug_kalimba','cd_get_logical_path', compile_unit,{''});
                    kcd.check_error(err);
                    symbols(i).compile_dir(j).compile_unit(k).path = cu_log_path{1};

                    % Read file globals from compile units
                    file_globals = libstruct('s_cd_variables_list',{});
                    err = calllib('core_debug_kalimba','cd_get_file_globals', compile_unit, file_globals);
                    kcd.check_error(err);
                    temp = obj.read_varlist(file_globals);
                    if ~isempty(temp)
                        symbols(i).compile_dir(j).compile_unit(k).globals = temp;
                        for n=1: numel(temp)/length(temp)
                            symbols(i).file_globals(glob_index,:) = [path, cu_log_path{1}, temp(n,:)];
                            glob_index = glob_index + 1;
                            obj.add_to_symbols(temp(n,1:3))
                        end
                    end
                    err = calllib('core_debug_kalimba', 'cd_free_variables', obj.cd, file_globals);
                    kcd.check_error(err);

                    % Read file statics from compile units
                    file_statics = libstruct('s_cd_variables_list',{});
                    err = calllib('core_debug_kalimba','cd_get_file_scope_statics', compile_unit, file_statics);
                    kcd.check_error(err);
                    temp = obj.read_varlist(file_statics);
                    if ~isempty(temp)
                        symbols(i).compile_dir(j).compile_unit(k).statics = temp;
                        for n=1: numel(temp)/length(temp)
                            symbols(i).file_statics(stat_index,:) = [path, cu_log_path{1}, temp(n,:)];
                            stat_index = stat_index + 1;
                            obj.add_to_symbols(temp(n,1:3))
                        end
                    end
                    err = calllib('core_debug_kalimba', 'cd_free_variables', obj.cd, file_statics);
                    kcd.check_error(err);

                    % Read function names from compile units
                    function_list_struct = libstruct('s_cd_function_list',{});
                    err = calllib('core_debug_kalimba', 'cd_get_functions', compile_unit, function_list_struct);
                    kcd.check_error(err);
                    function_list = calllib('core_debug_kalimba', 'cd_unpack_function_list', ...
                        function_list_struct);

                    % For each function in this compilation unit
                    functions = function_list_struct.number_of_functions;
                    for m = 1: functions
                        func = calllib('core_debug_kalimba', 'cd_access_function_element', ...
                            function_list, m-1);
                        [err, ~, fname] = calllib('core_debug_kalimba', 'cd_get_function_name', func, {''});
                        kcd.check_error(err);
                        symbols(i).compile_dir(j).compile_unit(k).functions(m).name = fname;
                        symbols(i).functions(func_index,:) = [path, cu_log_path{1}, fname(1)];
                        func_index = func_index + 1;

                        % Read function statics from functions
                        func_statics = libstruct('s_cd_variables_list',{});
                        err = calllib('core_debug_kalimba','cd_get_static_variables_from_function', func, func_statics);
                        kcd.check_error(err);
                        temp = obj.read_varlist(func_statics);
                        if ~isempty(temp)
                            symbols(i).compile_dir(j).compile_unit(k).functions(m).statics = temp;
                            for n=1: numel(temp)/length(temp)
                                symbols(i).func_statics(func_stat_index,:) = [path, cu_log_path{1}, temp(n,:)];
                                func_stat_index = func_stat_index + 1;
                                obj.add_to_symbols(temp(n,1:3))
                            end
                        end
                        err = calllib('core_debug_kalimba', 'cd_free_variables', obj.cd, file_statics);
                        kcd.check_error(err);

                    end
                    err=calllib('core_debug_kalimba', 'cd_free_functions', function_list_struct);
                    kcd.check_error(err);
                end
                err = calllib('core_debug_kalimba','cd_free_compile_units',compile_unit_list_struct);
                kcd.check_error(err);
            end
            err = calllib('core_debug_kalimba','cd_free_compile_dir_list',compile_dir_list_struct);
            kcd.check_error(err);
        end
    end

    %-Read-from-PM-memory--------------------------------------------------
    function vals = read_pm(obj, addrs, addr_per_word)
        vals = obj.ka.read_mems('ka_hal_read_pm_block', addrs, addr_per_word);
    end

    %-Read-from-DM-memory--------------------------------------------------
    function vals = read_dm(obj, addrs, addr_per_word)
        vals = obj.ka.read_mems('ka_hal_read_dm_block', addrs, addr_per_word);
    end

    %-Write-to-PM-memory---------------------------------------------------
    function write_pm(obj, addrs, vals, addr_per_word)
        obj.ka.write_mems('ka_hal_write_pm_block', addrs, vals, addr_per_word);
    end

    %-Write-to-DM-memory---------------------------------------------------
    function write_dm(obj, addrs, vals, addr_per_word)
        obj.ka.write_mems('ka_hal_write_dm_block', addrs, vals, addr_per_word);
    end

end % public instance methods

end % classdef
