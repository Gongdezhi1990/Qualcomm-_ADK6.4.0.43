function [ varargout ] = kalloadsym( filename, varargin )
%KALLOADSYM     Load the debug symbols for a Kalimba project.
% Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd
%
%   KALLOADSYM( FILENAME[.KLO|.ELF] ) parses the symbol information in
%   the specified file and stores it for the other Kalimba debug functions
%   to make use of. If FILENAME does not end with '.klo' or '.elf' then we
%   will search for both files and load the first one we find.
%
%   KALLOADSYM( DIR_NAME ) as above except that the specified directory
%   will be searched for .klo and .elf files. If more than one is present,
%   a list will be shown, so you can choose which one to load. If only one
%   is present, it will be loaded automatically.
%
%   KALLOADSYM( 'CLEAR' ) resets all kalimba symbol information
%
%   KALLOADSYM( 'UPDATE' ) refresh kalimba symbol information to
%   add or remove symbols which have fallen in or out of scope
%
%   See also kalprocessor, kalrunning, kalreadval, kalsymfind, kalwriteval.

% if called without any inputs display the help details
if nargin == 0
    help kalloadsym
    return
end

persistent core_debug
if isempty(core_debug)
    % Acquire a handle to the core_debug_kalimba.dll wrapper.
    core_debug.cd = kcd.instance();
end
cd = core_debug.cd;

if strcmpi(filename, 'CLEAR')
    cd.remove_symfiles();
    return
end

if strcmpi(filename, 'UPDATE')
    symb = kalvarprs;
    symb.symbols = cd.get_symbols();
    kalvarprs(symb);
    return
end

if cd.isConnected()
    % Discover the connected processor
    kalprocessor('FORCE');
else
    kalprocessor('UNKNOWN')
end

do_search = 1;

% did the user specify a .klo or .elf file directly?
if isempty(strfind(filename, '*'))
    [~, ~, ext] = fileparts(filename);
    ext = upper(ext);
    if strcmp(ext, '.KLO') || strcmp(ext, '.ELF')
        do_search = 0;
    end
end

% search for a klo or elf file if needed
if do_search
   filesFound = [];
   dirname = '';

   % did the user specify their own file glob?
   if not(isempty(strfind(filename, '*')))
       filesFound = dir(filename);
       if fileparts(filename)
          dirname = fileparts(filename);
       end
   end

   % search for filename.klo, filename.elf, filename/*.klo and filename/*.elf
   if size(filesFound, 1) == 0
       filesFound = dir([filename '.klo']);
       dirname = fileparts(filename);
   end
   if size(filesFound, 1) == 0
      filesFound = dir([filename '.elf']);
   end
   if size(filesFound, 1) == 0
      filesFound = dir([filename filesep '*.klo']);
      dirname = fileparts([filename filesep]);
   end
   if size(filesFound, 1) == 0
      filesFound = dir([filename filesep '*.elf']);
   end

   if size(filesFound, 1) == 0
      fprintf('No .klo or .elf files found\n')
      return
   elseif size(filesFound, 1) > 1
      fprintf('\n  Multiple files found:\n')

      for i = 1:size(filesFound, 1)
         fprintf('     [%d]  %s\n', i, filesFound(i).name)
      end

      n = input(['  Please select the file you wish to use, 1->' ...
                 num2str(size(filesFound, 1)) char(10)]);

      % check if anything was entered
      if isempty(n)
         warning('  No file was selected, the symbol table will not be updated.')
         return
      end
   else
      n = 1;
   end

   filename = fullfile(dirname, filesFound(n).name);
end

fprintf('  Loading %s\n', filename);

% initialise the symbols
symb.routine = {};
symb.symbols = {};
symb.kersymbols = {};
symb.constants = {};
symb.modules = {};
symb.dspinfo = struct();

[~, ~, ext] = fileparts(filename);
if strcmpi(ext, '.ELF')

    % Load the kalelfreader library is loaded, if required.
    ensureKalElfReaderLoaded();

    % Request kalelfreader to load the ELF file.
    %Do the void** passing in a particular way to a) avoid Matlab smushing the pointer's value for the next API call
    %and b) for Matlab to actually accept the code.
    %Basically, use the 1-param libpointer constructor form (no initializer), but later use setdatatype to inform Matlab
    %that "kerPtr" *contains* a void*. This seems to convince Matlab not to destroy the pointer.
    kerPtr = libpointer('voidPtr');
    kerErrPtr = calllib('kalelfreader', 'ker_open_file', filename, kerPtr);
    check_ker_error(kerErrPtr);
    clear kerErrPtr;
    setdatatype(kerPtr, 'voidPtr', 1); %We need to tell Matlab what it actually contains (i.e., a single void*), or .value will complain.
    ker = kerPtr.value;
    clear kerPtr;

    % Get the statements table.
    statementCount = calllib('kalelfreader', 'ker_statement_iter_init', ker);
    for i = 1:statementCount
        module = calllib('kalelfreader', 'ker_get_statement_module', ker);
        source_line = calllib('kalelfreader', 'ker_get_statement_source_line', ker);
        addr = calllib('kalelfreader', 'ker_get_statement_addr', ker);
        source_file = calllib('kalelfreader', 'ker_get_statement_source_file', ker);
        symb.modules(i, :) = { module, source_line, addr, source_file };
        calllib('kalelfreader', 'ker_statement_iter_next', ker);
    end

    % Get the variables table.
    variableCount = calllib('kalelfreader', 'ker_variable_iter_init', ker);
    for i = 1:variableCount
        name = calllib('kalelfreader', 'ker_get_variable_name', ker);
        siz = calllib('kalelfreader', 'ker_get_variable_size', ker);
        addr = calllib('kalelfreader', 'ker_get_variable_addr', ker);
        symb.kersymbols(i, :) = { name, siz, addr };
        calllib('kalelfreader', 'ker_variable_iter_next', ker);
    end

    % Get the constants table.
    constantCount = calllib('kalelfreader', 'ker_constant_iter_init', ker);
    for i = 1:constantCount
        name = calllib('kalelfreader', 'ker_get_constant_name', ker);
        value = calllib('kalelfreader', 'ker_get_constant_value', ker);
        symb.constants(i, :) = { name, 1, value };
        calllib('kalelfreader', 'ker_constant_iter_next', ker);
    end

    % Get the variables table.
    labelCount = calllib('kalelfreader', 'ker_label_iter_init', ker);
    for i = 1:labelCount
        name = calllib('kalelfreader', 'ker_get_label_name', ker);
        addr = calllib('kalelfreader', 'ker_get_label_addr', ker);
        symb.routine(i, :) = { name, 1, addr };
        calllib('kalelfreader', 'ker_label_iter_next', ker);
    end

    % Get info about the architecture this ELF file was built for.
    kerArch = libstruct('KerArchitecture', struct());
    kerErrPtr = calllib('kalelfreader', 'ker_get_architecture', ker, kerArch);
    check_ker_error(kerErrPtr);
    symb.dspinfo.addr_width = kerArch.addrWidth;
    symb.dspinfo.data_width = kerArch.dmDataWidth;
    symb.dspinfo.dm_byte_addressing = kerArch.dmByteAddressing;
    clear kerErrPtr;
    clear kerArch;

    % Tell kalelfreader it can release its ELF file data, we have local copies.
    calllib('kalelfreader', 'ker_close_file', ker); 
else
    file_open = fopen(filename);
    if file_open == -1
        error(['  Could not open ' filename]);
    end

    while ~feof(file_open)
        % read the line from the file
        file_line = fgets(file_open);
        % strip off unix or dos/windows line endings (unix has just a LF,
        % dos/windows has CR and LF), CR = 13, LF = 10
        if (length(file_line) >=2) && strcmp(file_line(end-1:end), char([13 10]))
            file_line = file_line(1:end-2);
        else
            file_line = file_line(1:end-1);
        end

        file_line_length = length(file_line);

        if file_line_length == 0
            continue
        end

        % check if we have a data line - will lead with "DEBUG"
        if strcmp(file_line(1:5), 'DEBUG')

            % knock off the 'DEBUG' at the beginning
            file_line = file_line(7:end);
            file_line_length = file_line_length - 6;

            % look for the space markers
            locat = [1, findstr(file_line, char(7)), file_line_length+1];

            % convert to words
            file_symbol = file_line(locat(1):(locat(2)-1));
            file_type = file_line(locat(2)+1:(locat(3)-1));
            file_size = str2num(file_line(locat(3)+1:(locat(4)-1)));
            file_address = str2num(file_line(locat(4)+1:(locat(5)-1)));

            % now determine where to store the data
            switch file_type
                case 'a'                            % program address data
                    symb.routine((size(symb.routine, 1)+1), :) = ...
                        {file_symbol, file_size+1, file_address};

                case 'v'                            % variable address data
                    symb.symbols((size(symb.symbols, 1)+1), :) = ...
                        {file_symbol, file_size, file_address};

                case 'c'                            % constant address data
                    symb.constants((size(symb.constants, 1)+1), :) = ...
                        {file_symbol, file_size+1, file_address};

                case 'l'                            % module Locations
                    % file_symbol is the module name
                    % file_size is the source file line number of the first
                    %  executable code in the module
                    % file_address is ???
                    % file_file is the source file name
                    file_file = file_line(locat(5)+1:(locat(6)-1));
                    symb.modules((size(symb.modules, 1)+1), :) = ...
                        {file_symbol, file_size, file_address, file_file};
            end
        end
    end

    % close the file
    fclose(file_open);
end

if cd.isConnected()
    % Get core debug to update the symbol list
    cd.add_symfile(filename);
    symb.symbols = cd.get_symbols();
else
    % Use kalelfreaders symbol list
    symb.symbols = symb.kersymbols;
end

% call the persistent function containing the variables
kalvarprs(symb);

if exist('kaloverlayselect')
    kaloverlayselect('reset');
end

% Inspect error struct from kalelfreader and throw an error if required.
function check_ker_error(kerErrorPtr)
    kerError = kerErrorPtr.value;
    if kerError.code ~= 0
        calllib('kalelfreader', 'ker_free_error', kerErrorPtr);
        error(strcat('error in kalelfreader: ', kerError.string));
    end
end

end % top-level function

function ensureKalElfReaderLoaded()
    if ~libisloaded('kalelfreader')
        [notfound, warnings] = loadlibrary('kalelfreader', @kalelfreader_proto);
        % Don't print the standard uninteresting 'warnings' relating to the thunk library and header, which are purely informational.
        % If the load failed, we get a proper error, in any case.
        if size(notfound) ~= [0,1]
            fprintf(notfound);
        end
    end
end