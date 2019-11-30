function [ varargout ] = kalspi( varargin )
% KALSPI provides information on, and connects to, attached Kalimba devices.
%   Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd 
%   
%
%   KALSPI() opens a dialog box for selecting and connecting to available
%   Kalimba devices.
%
%   KALSPI( 'OPEN', DEVICE_INDEX ) connects to a Kalimba by specifying its
%   one-based index in the list of available devices.
%
%   KALSPI( 'OPEN', TRANSPORT_STRING ) connects to a Kalimba by specifying a
%   transport string. This can either be in the traditional style, such as 
%   'SPITRANS=USB SPIPORT=0'
%   or using a QTIL IDE core URI, such as
%   device://trb/usb2trb/0/csra68100/app/p1
%   or
%   device://trb/usb2trb/0/csra68100/audio/p0
%
%   KALSPI( 'CLOSE' ) disconnects from the currently connected Kalimba.
%
%   KALSPI( 'RESET' ) disconnects from the connected Kalimba, if any, and
%   resets the persistent state in KALSPI.
%
%   KALSPI( 'STATE' ) lists the values of some common transport variables.
%   Requires a connection to a Kalimba.
%
%   KALSPI( 'GETARCH' ) returns or prints the architecture number of the connected
%   Kalimba.
%
%   KALSPI( 'GETCHIPNAME' ) returns or prints the chip name of the connected Kalimba.
%
%   KALSPI( 'GETCHIPREV' ) returns or prints the chip revision of the connected Kalimba.
%          
%   KALSPI( 'GETARCHFROMNAME', CHIPNAME ) returns or prints the architecture number of the
%   chip specified by CHIPNAME.
%
%   KALSPI( 'LISTDEVICES' ) lists the available devices in text format. This
%   provides the same information as the dialog box presented by KALSPI().
%
%   KALSPI( 'SETVAR', VARIABLE, VALUE ) sets the transport variable VARIABLE
%   to VALUE.
%
%   KALSPI( 'SETMUL', VALUE ) sets the SPI multiplex transport variable
%   to VALUE. This is a shortcut for KALSPI( 'SETVAR', 'SPIMUL', VALUE).
%
%   KALSPI( 'SETPORT', VALUE ) sets the SPI port transport variable
%   to VALUE. This is a shortcut for KALSPI( 'SETVAR', 'SPIPORT', VALUE).
%
%   KALSPI( 'REGID', REGNAME ) returns or prints the ID of the register specified in REGNAME.
%
%   KALSPI( 'READREG', REGID ) returns or prints the value of register whose ID is 
%   specified in REGID. It is usually preferable to use kalreadval(REGNAME) rather than this function.
%
%   KALSPI( 'RELEASEVERSION' ) returns the version string of this release of the Kalimba Matlab tools.
%   
% The other form of KALSPI provides low-level SPI access:
%   KALSPI ( ADDRESSES ) issues SPI reads for all addresses specified in ADDRESSES.
%
%   KALSPI ( ADDRESSES, VALUES ) issues SPI writes and/or reads. For items in VALUES whose
%   value is not NaN, a SPI write using that value is issued to the correspondingly-indexed element 
%   in ADDRESSES. For NaN values, a SPI read is issued, and the value read placed in the output
%   variable at the same index.
%
%   See also kalprocessor, kalreadval, kalwriteval, kalrunning.
%

% Persistent state.
persistent ks 
if isempty(ks)
    % Acquire a handle to the kalaccess.dll wrapper singleton.
    ks.ka = kxs.instance();
    ks.noConnectionError = 'No connection state, please connect..';
    ks.dialogHandle = [];
end
ka = ks.ka;

persistent core_debug
if isempty(core_debug)
    % Acquire a handle to the core_debug_kalimba.dll wrapper
    core_debug.cd = kcd.instance();
end
cd = core_debug.cd;


% If we've been passed no arguments, open a GUI and show the transport/device selection dialog box.
if (nargin == 0)

    % Refresh the current list of devices. Pre-select the first device on first-run.
    ka.refreshDevices();
    
    if length(ka.devices) == 0
        fprintf('No connected Kalimba devices detected.\n');
        return
    end
    
    % Create the dialog and set initial properties
    ks.dialogHandle = createDialog('Select device');
    set(ks.dialogHandle, ...
        'NumberTitle', 'off', ... % No figure number
        'MenuBar',     'None', ...    % No menu or toolbar -- only the controls we want.
        'ToolBar',     'None', ...
        'DeleteFcn',   'kalspi(''DIALOG_CALLBACK'', ''CLOSING'')', ...
        'Resize',      'off', ...
        'Color',        get(0, 'defaultUicontrolBackgroundColor'));
    
    % First-run
    if ~isfield(ks, 'selectedDevice')
        ks.selectedDevice = 1;
    end
    
    % If the last-chosen device has disappeared, reset. (selectedDevice could end up pointing to a different
    % device if the order has changed, but this is probably uncommon, and a minor annoyance anyway).
    if ks.selectedDevice > length(ka.devices)
        ks.selectedDevice = 1;
    end
    
    % Resize the dialog.
    % Position elements: [left, bottom, width, height]
    position = get(ks.dialogHandle, 'Position');
    position = [position(1), position(2), 800, 30 + 100 + 30 + 20 + 20];
    set(ks.dialogHandle, 'Position', position);
    
    % Device label.
    position = [20, position(4) - 40, 150, 30]; % 20 from (left, top).
    ks.fig.deviceLabel = uicontrol(ks.dialogHandle, ...
      'Style',               'Text', ...
      'Fontsize',            14, ...
      'HorizontalAlignment', 'left', ...
      'String',              'Select Device:', ...
      'Position',            position);
    
    deviceDisplayLabels = {};
    for i = 1:length(ka.devices)
        % If applicable, display the subsystem and processor information too. 
        % -1 corresponds to KaSubsysIdNone
        % Note that the device list is a 1 by n matrix -- hence the double indexing.
        if ka.devices(1, i).subsys_id ~= -1
            deviceDisplayLabels{i} = sprintf('%s: [Subsystem: %d, Processor: %d] (%s)', ...
                ka.devices(1, i).dongle_id, ...
                ka.devices(1, i).subsys_id, ka.devices(1, i).processor_id, ...
                ka.devices(1, i).transport_string);
        else
            deviceDisplayLabels{i} = sprintf('%s: (Transport string: %s)', ...
                ka.devices(1, i).dongle_id, ka.devices(1, i).transport_string);
        end        
    end

    % Device list.
    position = [20, position(2) - 100, 750, 100];
    ks.fig.deviceList = uicontrol(ks.dialogHandle, ...
      'Style',    'listbox', ...
      'Fontsize', 14, ...
      'String',   deviceDisplayLabels, ...
      'Value',    ks.selectedDevice, ...
      'Position', position, ...
      'Callback', 'kalspi(''DIALOG_CALLBACK'',''deviceList'')');
    
    % Connect button.
    position = [20, position(2) - 40, 120, 30];
    ks.fig.connectButton = uicontrol(ks.dialogHandle, ...
      'Style',    'PushButton', ...
      'String',   'Connect', ...
      'Fontsize', 14, ...
      'Position', position, ...
      'Callback', 'kalspi(''DIALOG_CALLBACK'', ''connectButton'')');
    
    % Diconnect button.
    position(1) = position(1) + position(3) + 20;
    ks.fig.disconnectButton = uicontrol(ks.dialogHandle, ...
      'Style',    'PushButton', ...
      'String',   'Disconnect', ...
      'Fontsize', 14, ...
      'Position', position, ...
      'Callback', 'kalspi(''DIALOG_CALLBACK'', ''disconnectButton'')');
      
    return
end

% When the user does something with the GUI - ie selects something in the    
% dialog box - then we get called back.
if strcmpi(varargin{1}, 'DIALOG_CALLBACK')
    switch upper(varargin{2})
        
      case 'DEVICELIST'
        % Select device. "gcbo" is the handle of the object whose callback this is!
        ks.selectedDevice = get(gcbo, 'Value');
        % This means it's a double-click (obviously...).
        if strcmp(get(ks.dialogHandle, 'SelectionType'), 'open')
            onConnectButton(ka, ks, cd);
        end
        
      case 'CONNECTBUTTON'
        onConnectButton(ka, ks, cd);

      case 'DISCONNECTBUTTON'
        ka.disconnect();
        fprintf('Disconnected\n');
        kalprocessor('UNKNOWN');

      case 'CLOSING'
        try
            ks = rmfield( ks, 'fig' );
            ks.dialogHandle = [];
        end
    end
    return
end


% If the first argument is a string its some sort of configuration command.
if ischar(varargin{1})
    
    % Assume it works - i.e. no error message.
    res = [];
    errRes = 0;
    errMsg = '';
    
    % Switch on the first argument.
    command = upper(varargin{1});
    switch (command)
      
      % Open a connection to kalaccess.
      case 'OPEN'
        destroyDialog('Select device');
        ka.refreshDevices();
        
        % If the second arg is not a number of a string representation of a number, use 
        % connectGivenString(), which will see if it's a core URI or an old-style transport
        % string. Othewise, connect using a device index.
        deviceOrTransportString = varargin{2};
        if ~isnumeric(deviceOrTransportString) && isempty(str2num(deviceOrTransportString))
            [errRes, errMsg] = connectGivenString(ka, deviceOrTransportString);
        else
            deviceIndex = deviceOrTransportString;
            if ischar(deviceIndex)
                deviceIndex = str2num(deviceIndex);
            end
            ks.selectedDevice = deviceIndex;
            [errRes, errMsg] = connectGivenDeviceIndex(ka, deviceIndex);
        end
        
      % Close a kalaccess connection.
      case 'CLOSE'
        if ka.isConnected()
            ka.disconnect();
        end
        if cd.isConnected()
            cd.disconnect();
        end
        fprintf('Disconnected\n');
        kalprocessor('UNKNOWN');

      % Close any active session and clear local state.
      case 'RESET'
        if ka.isConnected()
            ka.disconnect();
        end
        if cd.isConnected()
            cd.disconnect();
        end
        close(ks.dialogHandle); % close the dialog
        ks = [];

      % Print the state: list values of common transport variables.
      case 'STATE'
        vars = { 'SPITRANS'; 'BABELMODE'; 'SPIUSBDEV'; 'SPIPORT'; ...
                 'SPI_DELAY'; 'SPICMDBITS'; 'SPIMUL'; 'SPI_DELAY_MODE' };
        res = struct;
        for i=1:length(vars)
            var = char(vars{i});
            str = ka.trans_get_var(var);
            res.(var) = str;
        end

      case 'GETARCH'
        res = ka.get_arch();
          
      case 'GETCHIPNAME'
        res = ka.get_chip_name();
       
      case 'GETCHIPREV'
        res = ka.get_chip_rev();
          
      case 'GETARCHFROMNAME'
        res = ka.get_arch_from_name(lower(varargin{2}));
       
      case 'LISTDEVICES'
        ka.refreshDevices();
        ka.listDevices();

      case 'SETVAR'
        ka.trans_set_var(varargin{2,3});

      case 'SETMUL'
        ka.trans_set_var('SPIMUL',varargin{2});

      case 'SETPORT'
        ka.trans_set_var('SPIPORT',varargin{2});

      case 'REGID'
        % We expect a valid register name.
        assert(nargin >= 2, 'Specify a register name');
        assert(ischar(varargin{2}), 'Register name must be a valid string.');
        
        regName = upper(char(varargin(2)));
        regId = ka.get_register_id(regName);
        
        if regId == kxs.registerIdNoSuchRegister
            errRes = 1;
            errMsg = 'Register not present on this architecture.';
        else
            res = regId;
        end
        
      case 'READREG'
        % We expect a valid register id.
        assert(nargin >= 2, 'Specify a register id');
        assert(isnumeric(varargin{2}), 'Register id must be numeric');
        
        regId = varargin{2};
        res = ka.read_register(regId);
        
      case 'RELEASEVERSION'
        res = '1.1.7';
      
      otherwise
        % We don't understand this command.
        errRes = 1;
        errMsg = ['Unknown argument ' varargin{1}];

    end % switch

    % Is an output required?
    if (nargout == 0)
        if (errRes == 1)
            error(errMsg);  % show an error
        end
        if ~isempty( res )
            disp(res);      % if we have results then show them
        end
    else
        if (errRes == 1)
            varargout{1} = 1;
            if (nargout == 2)
                varargout{2} = errMsg;
            end
        else
            varargout{1} = res; % pass back the result
        end
    end

else
    switch (nargin)
      case 1
        % 1 parameter means an address or array of addresses to read
        assert(ka.isConnected(), ks.noConnectionError);
        addrs = varargin{1};
        vals = zeros(1,length(addrs),'uint16');
        for i=1:length(addrs)
            vals(:,i) = ka.spi_read(addrs(:,i));
        end
      
      case 2
        % Two parameters allows writes and/or reads. The first parameter is
        % the address or array of addresses. The second parameter is a value or
        % array of values. For elements in the value array which are not
        % NaN, a SPI write is performed using the value. For elements which are
        % NaN, a SPI *read* is performed, and the value returned at
        % the corresponding index in the results array. This allows a mix of
        % SPI reads and writes to be performed in a single call.
        addrs = varargin{1};
        vals = varargin{2};
        if length(addrs) ~= length(vals)
            error('Supply the same number of data values as address values');
        end
        for i=1:length(addrs)
            if (isnan(varargin{2}(i)))
                vals(:,i) = ka.spi_read(addrs(:,i));
            else
                ka.spi_write(addrs(:,i), vals(:,i));
            end
        end
      
      otherwise
        error('Incorrect number of arguments to kalspi');
    end % switch
    
    % Do we need an output?
    if (nargout == 1)
        varargout{1} = vals;
        clear temp;
    else
        disp(vals);
    end
    
end % if ischar(varargin{1})
end % function

function [ hnd ] = createDialog( name )
    % Create a figure with the given name, after deleting any other figures
    % with the same name.
    destroyDialog(name)
    hnd = figure('Name', name);
end

function destroyDialog( name )
    % Delete all figures with the given name.
    hRootGfxObj = 0;
    kids = get(hRootGfxObj,'children');
    for i =1:length(kids)
        kid = kids(i);
        if strcmp(get(kid,'Name'), name)
            close(kid);
        end
    end
end

function onConnectButton(ka, ks, cd)
    close(ks.dialogHandle) % close the dialog
    if ka.isConnected()
        ka.disconnect(); % close an old session
    end
    if cd.isConnected()
        cd.disconnect(); % close an old session
    end
    ka.connect(ks.selectedDevice); % open the new session
    fprintf('Connected OK on %s\n', ka.devices(ks.selectedDevice).transport_string);
    kalprocessor('FORCE');
end

function [errRes, errMsg] = connectGivenString(ka, transportStringOrURI)
    try
        if strfind(transportStringOrURI, 'device://') == 1
            ka.connect_via_core_uri(transportStringOrURI);
        else
            ka.connect_via_transport_string(transportStringOrURI);
        end
        fprintf('Connected OK on %s\n', transportStringOrURI);
        kalprocessor('FORCE'); % force kalprocessor refresh
        errRes = 0;
        errMsg = '';
    catch exception
        errRes = 1;
        errMsg = sprintf('Cannot connect to a device with the supplied settings:\n%s', exception.message);
    end
end

function [errRes, errMsg] = connectGivenDeviceIndex(ka, deviceIndex)
    try
        ka.connect(deviceIndex);
        fprintf('Connected OK\n');
        kalprocessor('FORCE'); % force kalprocessor refresh
        errRes = 0;
        errMsg = '';
    catch exception
        errRes = 1;
        errMsg = sprintf('Cannot connect to a device with the supplied settings:\n%s', exception.message);
    end
end
