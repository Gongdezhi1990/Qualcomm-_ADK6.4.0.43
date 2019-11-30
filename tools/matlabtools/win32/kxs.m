%##########################################################################
% Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd 

classdef kxs < handle
  % KXS Wrapper class for kalaccess.dll.
    
%=Constant=properties====================================================
properties (Constant)
    verbose = false; % true;
    noConnectionMessage = 'No connection state, please connect..';
    writeLengthMismatchMessage = 'must have the same number of values as addresses';
    accessNoWordsMessage = 'Cannot access zero words of memory from the DSP';
    invalidDeviceMessage = 'Invalid device specified';
    registerIdNoSuchRegister = hex2dec('ffff');
    
    % For ka_get_state. Source: kalaccess. 
    KA_STATE_PM_BREAK = 0;
    KA_STATE_DM_BREAK = 1;
    KA_STATE_RUNNING  = 2;
    KA_STATE_STOPPED  = 3;
    KA_STATE_EXCEPTION_BREAK = 4;
    KA_STATE_EXTERNAL_BREAK  = 5;
    KA_STATE_CLOCK_OR_POWER_OFF = 6;
end
    
%=Properties=============================================================
properties (SetAccess=private)
    ka;
    connectString;
    coreUri;
    registerNameMap;
    isConnected = false;
    deviceTablePtr = 0;
    devices = [];
end
  
%=Private=methods========================================================
methods(Access=private)
    
    %-Private-singleton-constructor:-must-go-via-instance()----------------
    function newObj = kxs()
      newObj.ka = 0;
    end
    
    %-Read-from-PM-or-DM-memory--------------------------------------------
    function outputValues = read_mems(obj, dllFunction, addresses, addr_per_word)
    
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        
        numAddresses = length(addresses);
        assert(numAddresses ~= 0, kxs.accessNoWordsMessage)
              
        % Block-read contiguous address ranges.
        firstAddress      = addresses(1);
        outputValues      = zeros(1, numAddresses);
        outputValuesIndex = 1;
        currentReadSize   = 1;
                
        % We've consumed the first address, so start at 2. Loop to one-beyond the end so as to consume
        % all addresses and read them in contiguous blocks if possible.
        for i = 2:numAddresses + 1
            if (i > numAddresses || addresses(i) ~= firstAddress + currentReadSize*addr_per_word)
                % End of this block; read it.
                
                % Go via an extra temporary buffer
                tempOutputBuffer        = uint32(zeros(1, currentReadSize));
                tempOutputBufferPointer = libpointer('uint32Ptr', tempOutputBuffer);
        
                dllErrorPointer = calllib('kalaccess', dllFunction, obj.ka, uint32(firstAddress), tempOutputBufferPointer, currentReadSize);
                kxs.checkError(dllErrorPointer);
                clear dllErrorPointer;
                
                outputValues(outputValuesIndex : outputValuesIndex + currentReadSize  - 1) = tempOutputBufferPointer.value;
                clear tempOutputBufferPointer;
                
                outputValuesIndex = outputValuesIndex + currentReadSize;
                
                % We're all done
                if i > numAddresses
                    break;
                end
                
                % Otherwise, the next read should start from this point
                firstAddress = addresses(i);
                currentReadSize = 1;
            else
                currentReadSize = currentReadSize + 1;
            end
        end
        
    end
    
    %-Write-to-PM-or-DM-memory---------------------------------------------
    function write_mems(obj, dllFunction, addresses, values, addr_per_word)
    
        assert(obj.ka ~= 0, kxs.noConnectionMessage);

        numAddresses = length(addresses);
        assert(numAddresses ~= 0,              kxs.accessNoWordsMessage)
        assert(length(values) == numAddresses, kxs.writeLengthMismatchMessage);

        % Block-write contiguous address ranges.
        firstAddress     = addresses(1);
        currentIndex     = 1;
        currentWriteSize = 1;
      
        for i = 2:numAddresses + 1
            if (i > numAddresses || addresses(i) ~= firstAddress + currentWriteSize*addr_per_word)
                
                tempBuffer = uint32(values(currentIndex : currentIndex + currentWriteSize - 1));
                tempBufferPointer = libpointer('uint32Ptr', tempBuffer);
                 
                dllErrorPointer = calllib('kalaccess', dllFunction, obj.ka, uint32(firstAddress), tempBufferPointer, currentWriteSize);
                kxs.checkError(dllErrorPointer);
                clear dllErrorPointer;
                
                clear tempBufferPointer;
                currentIndex = currentIndex + currentWriteSize;
          
                % We're all done
                if i > numAddresses
                    break;
                end
                
                % Otherwise, the next write should start from this point
                firstAddress = addresses(i);
                currentWriteSize = 1;
            else
                currentWriteSize = currentWriteSize + 1;
            end
        end
    end
  
    function pre_connect(obj)
        if obj.ka~=0
            obj.disconnect();
        end
        if ~libisloaded('kalaccess')
            obj.load();
        end
        obj.coreUri = '';
    end
    
    function post_connect(obj, kaPtr)
        % We need to tell Matlab what it actually contains (i.e., a single void*), or accessing
        % ".value" will complain.
        setdatatype(kaPtr, 'voidPtr', 1);
        obj.ka = kaPtr.value;
    end
end
  
%=Public=static=methods==================================================
methods(Static, Access=public)
 
    %-Retrieve-the-singleton-instance--------------------------------------
    function obj = instance()
        persistent uniqueInstance
        if isempty(uniqueInstance)
            obj = kxs();
            obj.load();
            uniqueInstance = obj;
        else
            obj = uniqueInstance;
        end
    end
        
    %-Unit-test------------------------------------------------------------
    function test()
        k = kxs.instance();
        k.load();
        k.refreshDevices();
        k.connect(0);
        k.isConnected();
        k.disconnect();
        k.unload();
    end
    
    %-Get-dsp-revision-from-name-------------------------------------------
    function rev = get_arch_from_name(str)
        rev = calllib('kalaccess', 'ka_get_arch_from_name', str);
    end
    
end

%=Properties=methods=====================================================
methods
    
    %-Is-there-an-active-session-with-the-DLL?-----------------------------
    function value = get.isConnected(obj)
        value = obj.ka ~= 0;
    end
    
end
  
%=Private=static=methods=================================================
methods(Static, Access=private)
    
    %-Check-error-struct-returned-from-the-DLL-----------------------------
    function checkError(errPtr)
        err = errPtr.value;
        if err.err_code ~= 0
            calllib('kalaccess','ka_free_error',errPtr);
            error(err.err_string);
        end
        clear errPtr;
    end
        
    %-If-in-verbose-mode-then-log-messages-to-the-console------------------
    function log(str)
        if kxs.verbose
            fprintf('kxs %s\n', str);
        end
    end
        
end
        
%=Public=instance=methods================================================
methods(Access = public)
    
    %-Load-the-DLL-and-read-the-register-names-----------------------------
    function load(obj)

        % Re/load the DLL.
        if libisloaded('kalaccess')
            unloadlibrary('kalaccess')
            kxs.log('dll unload');
        end
        loadlibrary('kalaccess', @kalaccess_proto);
        kxs.log('dll load');
      
        % Get register names.
        obj.registerNameMap = containers.Map();
        ptr = calllib('kalaccess','ka_get_register_names');
        i = 0;
        while 1
            regName = char(ptr.value);
            if strcmp(regName,'NumRegs')
                break
            end
            ptr = ptr + 1;
            obj.registerNameMap(regName) = i;
            i = i + 1;
        end
        clear ptr;
        
    end
    
    %-Delete-the-register-names-and-unload-the-dll-------------------------
    function unload(obj)
        remove(obj.registerNameMap, obj.registerNameMap.keys);
        if obj.ka ~= 0
            obj.disconnect();
        end
        unloadlibrary('kalaccess');
        kxs.log('dll unload');
    end
    
    %-Connect-to-the-specified device--------------------------------------
    function connect(obj, deviceIndex)
        assert(deviceIndex > 0,                    kxs.invalidDeviceMessage)
        assert(deviceIndex <= length(obj.devices), kxs.invalidDeviceMessage)
        
        obj.pre_connect();
        dev = obj.devices(deviceIndex);
        obj.connectString = dev.transport_string;

        kaPtr = libpointer('voidPtr');
        deviceIndexForC = deviceIndex - 1; % Convert one-based index to zero-based index.
        errPtr = calllib('kalaccess', 'ka_connect', obj.deviceTablePtr + deviceIndexForC, true, kaPtr);
        kxs.checkError(errPtr);

        kxs.log(['ka connect device ', num2str(deviceIndex)]);
        obj.post_connect(kaPtr);
            
        if strncmp('trb/',dev.transport_string,4)
            transport = strcat('device://', dev.transport_string,'/');
        elseif strncmp('tc/',dev.transport_string,3)
            transport = strcat('device://', dev.transport_string,'/');
        elseif strncmp('SPITRANS=USB',dev.transport_string,12)
            transport = regexprep(dev.dongle_id, ...
                '^USB SPI \((.*)\)$', ...
                'device://spi/usb2spi/$1/');
        elseif strncmp(dev.dongle_id,'kalsim',6)
            transport = regexprep(dev.dongle_id, ...
                '^kalsim (.*)$', ...
                'device://tcp/$1/spi/sim/');
        else
            % error('Unable to parse transport format');
            transport = dev.transport_string;
        end

        chipname = obj.get_chip_name();
        % strip e.g. "_audio" from chipname
        chipname = regexprep(chipname,'_.*$','');

        switch dev.subsys_id
            case 4
                subsystem = '/app';
            otherwise
                subsystem = '/audio';
        end
        
        processor = {'/p0','/p1','/p2','/p3'};
        
        obj.coreUri = strcat(transport, chipname, subsystem);

        if dev.subsys_id >= 0
            obj.coreUri = strcat(obj.coreUri, char(processor(dev.processor_id + 1)));
            
            k = kcd.instance();
            k.connect(obj.coreUri);
            clear k;
        end
    end
    
    %-Connect-via-a-Heracles-core-uri--------------------------------------
    function connect_via_core_uri(obj, coreUriString)
        assert(ischar(coreUriString))
        
        obj.pre_connect();
        obj.connectString = coreUriString;
        
        kaPtr = libpointer('voidPtr');
        errPtr = calllib('kalaccess', 'ka_connect_uri', coreUriString, kaPtr);
        kxs.checkError(errPtr);
        
        obj.coreUri = coreUriString;

        kxs.log(['ka connect (via core URI) ', coreUriString]);
        obj.post_connect(kaPtr);
    end
    
    %-Connect-via-a-raw-transport-string-----------------------------------
    function connect_via_transport_string(obj, transportString)
        assert(ischar(transportString))
        
        obj.pre_connect();
        obj.connectString = transportString;
        
        kaPtr = libpointer('voidPtr');
        errPtr = calllib('kalaccess', 'ka_connect_simple', transportString, true, kaPtr);
        kxs.checkError(errPtr);

        obj.coreUri = obj.connectString;
        kxs.log(['ka connect (via transport string) ', transportString]);
        obj.post_connect(kaPtr);
    end
    
    %-Disconnect-an-active-session-----------------------------------------
    function disconnect(obj)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        calllib('kalaccess','ka_disconnect',obj.ka);
        kxs.log('ka disconnect');
        obj.ka = 0;
    end
    
    %-Free-a-previously-allocated-device-table-----------------------------
    function freeDeviceTable(obj)
        if obj.deviceTablePtr ~= 0
            calllib('kalaccess', 'ka_trans_free_device_table', obj.deviceTablePtr, length(obj.devices));
            kxs.log('Device table freed');
            obj.deviceTablePtr = 0;
        end
    end
    
    %-Get-the-devices-table------------------------------------------------
    function refreshDevices(obj)
        % Don't need a ka but the dll must be loaded.
        if ~libisloaded('kalaccess')
            obj.load();
        end
        
        % Free the table from the last query, if any exists.
        obj.freeDeviceTable();
      
        % Get the list of available devices from kalaccess.
        deviceCountPtr = libpointer('int32Ptr', 0);
        % I presume this is necessary for Matlab to know the size of the objects in the device table, or something..
        dummyDeviceObject  = libstruct('ka_connection_details'); 
        obj.deviceTablePtr = libpointer('ka_connection_detailsPtr', dummyDeviceObject);
        
        errPtr = calllib('kalaccess', 'ka_trans_build_device_table', obj.deviceTablePtr, deviceCountPtr);
        kxs.checkError(errPtr);
        
        deviceCount = deviceCountPtr.value;
        clear deviceCountPtr;
        clear dummyDeviceObject;
        kxs.log(sprintf('Device table built with %d devices', deviceCount));

        % Process the returned device table. For now, we just copy the entries.
        % The raw device table pointer is kept around for later use by ka_connect.
        obj.devices = [];
        currentDevicePtr = obj.deviceTablePtr;
        for i = 1:deviceCount
            device = currentDevicePtr.value;
            obj.devices = [ obj.devices, device ];
            if i < deviceCount % or Matlab will crash!
                currentDevicePtr = currentDevicePtr + 1;
            end
        end
        clear currentDevicePtr;
    end
    
    %-List-device-transports-----------------------------------------------
    function listDevices(obj)
        for device = obj.devices
            fprintf('======== Device ========\nDebug device: %s\nTransport string: %s\nSubsystem ID: %d\nProcessor ID: %d\n', ...
                    device.dongle_id, device.transport_string, device.subsys_id, device.processor_id);
        end
    end
    
    %-Get-transport-variable-----------------------------------------------
    function str = trans_get_var(obj, var)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        str = calllib('kalaccess','ka_trans_get_var',obj.ka,var);
    end
    
    %-Set-transport-variable-----------------------------------------------
    function trans_set_var(obj, var, str)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        calllib('kalaccess','ka_trans_set_var',obj.ka,var,str);
    end
    
    %-Get-register-id-from-name--------------------------------------------
    function regId = get_register_id(obj, regName)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        assert(obj.registerNameMap.isKey(regName), 'Unknown register name.');

        regId = calllib('kalaccess', 'ka_get_register_id', obj.ka, regName);
    end

    %-Read-register--------------------------------------------------------
    function val = read_register(obj,regid)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        ru32 = uint32(regid);
        v32 = uint32(0);
        pv32 = libpointer('uint32Ptr', v32);
        errPtr = calllib('kalaccess', 'ka_hal_read_register', obj.ka, ru32, pv32);
        kxs.checkError(errPtr);
        clear errPtr;
        val = double(pv32.value);
        clear pv32;
    end

    %-Write-register-------------------------------------------------------
    function write_register(obj,regid,val)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        ru32 = uint32(regid);
        v32 = uint32(val);
        errPtr = calllib('kalaccess', 'ka_hal_write_register', obj.ka, ru32, v32);
        kxs.checkError(errPtr);
        clear errPtr;
    end

    %-Get-register-width---------------------------------------------------
    function width = register_width(obj,regid)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        ru32 = uint32(regid);
        w32 = calllib('kalaccess', 'ka_get_register_width', obj.ka, ru32);
        width = w32;
    end

    %-Get-architecture-revision--------------------------------------------
    function arch = get_arch(obj)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        arch = calllib('kalaccess', 'ka_get_arch', obj.ka);
    end
    
    %-Get-chip-name--------------------------------------------------------
    function str = get_chip_name(obj)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        str = calllib('kalaccess', 'ka_get_chip_name', obj.ka);
    end
    
    % Check if the supplied name is a match for the name of connected chip or one of its aliases.
    function is_match = check_chip_name(obj, name)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        is_match = calllib('kalaccess', 'ka_check_chip_name', obj.ka, name);
    end
    ka_check_chip_name
    
    %-Get-dsp-revision-----------------------------------------------------
    function rev = get_chip_rev(obj)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        rev = calllib('kalaccess', 'ka_get_chip_rev', obj.ka);
    end
    
    %-Read-from-the-SPI-memory-map-----------------------------------------
    function val = spi_read(obj,addr)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        a32 = uint32(addr);
        v16 = uint16(0);
        vPtr = libpointer('uint16Ptr',v16);
        errPtr = calllib('kalaccess', 'ka_hal_spi_read', obj.ka, a32, vPtr);
        val = vPtr.value;
        kxs.checkError(errPtr);
        clear errPtr;
    end
    
    %-Write-to-the-SPI-memory-map------------------------------------------
    function spi_write(obj, addr, val)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        a32 = uint32(addr);
        v16 = uint16(val);
        errPtr = calllib('kalaccess', 'ka_hal_spi_write', obj.ka, a32, v16);
        kxs.checkError(errPtr);
        clear errPtr;
    end
    
    %-Read-from-PM-memory--------------------------------------------------
    function vals = read_pm(obj, addrs, addr_per_word)
        vals = obj.read_mems('ka_hal_read_pm_block', addrs, addr_per_word);
    end
    
    %-Read-from-DM-memory--------------------------------------------------
    function vals = read_dm(obj, addrs, addr_per_word)
        vals = obj.read_mems('ka_hal_read_dm_block', addrs, addr_per_word);
    end
    
    %-Write-to-PM-memory---------------------------------------------------
    function write_pm(obj, addrs, vals, addr_per_word)
        obj.write_mems('ka_hal_write_pm_block', addrs, vals, addr_per_word);
    end
    
    %-Write-to-DM-memory---------------------------------------------------
    function write_dm(obj, addrs, vals, addr_per_word)
        obj.write_mems('ka_hal_write_dm_block', addrs, vals, addr_per_word);
    end
    
    %-Set-the-processor-running--------------------------------------------
    function ka_run(obj)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        dllErrorPointer = calllib('kalaccess', 'ka_hal_run', obj.ka);
        kxs.checkError(dllErrorPointer);
        clear dllErrorPointer;
    end
    
    %-Pause-the-processor--------------------------------------------------
    function ka_pause(obj)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        dllErrorPointer = calllib('kalaccess', 'ka_hal_pause', obj.ka);
        kxs.checkError(dllErrorPointer);
        clear dllErrorPointer;
    end
    
    %-Step-the-processor---------------------------------------------------
    function ka_step(obj)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        dllErrorPointer = calllib('kalaccess', 'ka_hal_step', obj.ka);
        kxs.checkError(dllErrorPointer);
        clear dllErrorPointer;
    end

    %-Get-processor-state--------------------------------------------------
    function state = ka_get_state(obj)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        
        statePointer = libpointer('int32Ptr', 0);
        
        dllErrorPointer = calllib('kalaccess', 'ka_get_chip_state', obj.ka, statePointer);
        kxs.checkError(dllErrorPointer);
        clear dllErrorPointer;
        
        state = statePointer.value;
        clear statePointer;
    end
    
    function numBreakpoints = get_num_pm_breakpoints(obj)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        numBreakpoints = calllib('kalaccess', 'ka_hal_get_max_pm_breakpoints', obj.ka);
    end
    
    function numBreakpoints = get_num_dm_breakpoints(obj)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        numBreakpoints = calllib('kalaccess', 'ka_hal_get_max_dm_breakpoints', obj.ka);
    end
    
    function [breakpointSet, breakpointAddress] = get_pm_breakpoint(obj, breakpointNumber)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        
        tempAddress = uint32(0);
        addressPointer = libpointer('uint32Ptr', tempAddress);
        tempEnabled = logical(0);
        enabledPointer = libpointer('bool', tempEnabled);
        
        dllErrorPointer = calllib('kalaccess', 'ka_hal_get_pm_breakpoint', obj.ka, uint32(breakpointNumber), addressPointer, enabledPointer);
        kxs.checkError(dllErrorPointer);
        clear dllErrorPointer;
        
        breakpointSet = enabledPointer.value;
        breakpointAddress = addressPointer.value;
        clear enabledPointer;
        clear addressPointer;
    end
    
    function [startAddress, endAddress, triggerOnRead, triggerOnWrite] = get_dm_breakpoint(obj, breakpointNumber)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        
        tempStartAddress = uint32(0);
        startAddressPointer = libpointer('uint32Ptr', tempStartAddress);
        
        tempEndAddress = uint32(0);
        endAddressPointer = libpointer('uint32Ptr', tempEndAddress);
        
        tempTriggerOnRead = logical(0);
        triggerOnReadPointer = libpointer('bool', tempTriggerOnRead);
        
        tempTriggerOnWrite = logical(0);
        triggerOnWritePointer = libpointer('bool', tempTriggerOnWrite);
        
        dllErrorPointer = calllib('kalaccess', 'ka_hal_get_dm_breakpoint', obj.ka, uint32(breakpointNumber), ...
                                  startAddressPointer, endAddressPointer, triggerOnReadPointer, triggerOnWritePointer);
        kxs.checkError(dllErrorPointer);
        clear dllErrorPointer;
        
        startAddress   = startAddressPointer.value;
        endAddress     = endAddressPointer.value;
        triggerOnRead  = triggerOnReadPointer.value;
        triggerOnWrite = triggerOnWritePointer.value;
        
        clear startAddressPointer;
        clear endAddressPointer;
        clear triggerOnReadPointer;
        clear triggerOnWritePointer;
    end
    
    function clear_all_pm_breakpoints(obj)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
    
        dllErrorPointer = calllib('kalaccess', 'ka_hal_clear_all_pm_breakpoints', obj.ka);
        kxs.checkError(dllErrorPointer);
        clear dllErrorPointer;
    end
    
    function clear_all_dm_breakpoints(obj)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
    
        dllErrorPointer = calllib('kalaccess', 'ka_hal_clear_all_dm_breakpoints', obj.ka);
        kxs.checkError(dllErrorPointer);
        clear dllErrorPointer;
    end
    
    function clear_pm_breakpoint(obj, breakpointNumber)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        
        dllErrorPointer = calllib('kalaccess', 'ka_hal_clear_pm_breakpoint', obj.ka, uint32(breakpointNumber));
        kxs.checkError(dllErrorPointer);
        clear dllErrorPointer;
    end
    
    function clear_dm_breakpoint(obj, breakpointNumber)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        
        dllErrorPointer = calllib('kalaccess', 'ka_hal_clear_dm_breakpoint', obj.ka, uint32(breakpointNumber));
        kxs.checkError(dllErrorPointer);
        clear dllErrorPointer;
    end
    
    function set_pm_breakpoint(obj, breakpointNumber, breakpointAddress)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
    
        dllErrorPointer = calllib('kalaccess', 'ka_hal_set_pm_breakpoint', obj.ka, uint32(breakpointNumber), uint32(breakpointAddress));
        kxs.checkError(dllErrorPointer);
        clear dllErrorPointer;
    end
    
    function set_dm_breakpoint(obj, breakpointNumber, startAddress, endAddress, triggerOnRead, triggerOnWrite)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
    
        dllErrorPointer = calllib('kalaccess', 'ka_hal_set_dm_breakpoint', obj.ka, uint32(breakpointNumber),...
                                  uint32(startAddress), uint32(endAddress), triggerOnRead, triggerOnWrite);
        kxs.checkError(dllErrorPointer);
        clear dllErrorPointer;
    end
    
    function sampleData = pcprofile(obj, numSamples)
        assert(obj.ka ~= 0, kxs.noConnectionMessage);
        
        sampleData    = zeros(1, numSamples);
        sampleDataPtr = libpointer('uint32Ptr', sampleData);
        
        dllErrorPointer = calllib('kalaccess', 'ka_hal_pcprofile', obj.ka, sampleDataPtr, uint32(numSamples));
        kxs.checkError(dllErrorPointer);
        clear dllErrorPointer;
        
        sampleData = sampleDataPtr.value;
        clear sampleDataPtr;
    end
    
end % public instance methods
  
end % classdef
