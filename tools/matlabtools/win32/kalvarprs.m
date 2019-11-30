function [ varargout ] = kalvarprs( varargin )
%KALVARPRS store and retreive the Kalimba symbol table.
% Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd 
% 
%   [SYMB] = KALVARPRS returns the complete list of symbols.
%
%   [SYMBPM SYMBCONST SYMBDM SYMBMODULE DSPINFO] = KALVARPRS returns each region's 
%   symbols separately.
%
%   [SYMB] = KALVARPRS( REGION ) returns the specified region's symbols.
%
%   KALVARPRS( SYMB ) updates the complete list of symbols. This should normally
%   be done by calling kalloadsym, rather than directly.
%
%   KALVARPRS( SYMB, REGION ) updates the specified region's list of symbols.
%   This should normally be done by calling kalloadsym, rather than directly.
%
%   NOTE: users are not intended to make use of this function to update the symbol
%   information. It is used by the other Kalimba Matlab tools as part of symbolic
%   operations. Modifying the symbol table directly using this function may lead
%   to erroneous results when using the other tools.
%
%   See also kalloadsym, kalsymfind, kalreadval, kalwriteval, kalrunning.
%

persistent KAL_SYMB

noSymbolsError = 'Symbol information not available; call kalloadsym to load symbols.';
badRegionError = ['You have not selected a valid memory region. ' ...
                 'Please select from PM, DM, CONST, MODULE, or DSPINFO; see KALLOADSYM'...
                 ' help for details'];
switch nargin
    case 0
        if isempty(KAL_SYMB)
            error(noSymbolsError);
        end

        % This is either help or "return the symbols"
        switch nargout
            case 0 % Display help screen
                help kalvarprs
                return;
            case 1
                % Return the whole structure
                varargout{1} = KAL_SYMB;
            case 5
                % Return each class individually
                varargout{1} = KAL_SYMB.routine;
                varargout{2} = KAL_SYMB.constants;
                varargout{3} = KAL_SYMB.symbols;
                varargout{4} = KAL_SYMB.modules;
                varargout{5} = KAL_SYMB.dspinfo;
            otherwise
                error('Invalid number of output arguments');
        end

    case 1
        if (isstr(varargin{1})) 
            % If it's a string, then this is the specifier for the output required
            
            if isempty(KAL_SYMB)
                error(noSymbolsError);
            end
            
            switch (upper(varargin{1}))
                case 'PM'                                   % Program address data
                    varargout{1} = KAL_SYMB.routine;
                case 'DM'                                   % Variable address data
                    varargout{1} = KAL_SYMB.symbols;
                case 'CONST'                                % Constant address data
                    varargout{1} = KAL_SYMB.constants;
                case 'MODULE'                               % Module location data
                    varargout{1} = KAL_SYMB.modules;
                case 'DSPINFO'                              % Dsp Info data
                    varargout{1} = KAL_SYMB.dspinfo;
                otherwise
                    error(badRegionError);
            end

        else
            % This is the complete symbol class
            KAL_SYMB = varargin{1};
        end

    case 2
        % This means one of the variable types has been passed to be updated
        switch (upper(varargin{2}))
            case 'PM'                                       % Program address data
                KAL_SYMB.routine = varargin{1};
            case 'DM'                                       % Variable address data
                KAL_SYMB.symbols = varargin{1};
            case 'CONST'                                    % Constant address data
                KAL_SYMB.constants = varargin{1};
            case 'MODULE'                                   % Module location data
                KAL_SYMB.modules = varargin{1};
            case 'DSPINFO'                                  % Dsp Info data
                KAL_SYMB.dspinfo = varargin{1};
            otherwise
                error(badRegionError);
        end
end % switch

end % top-level function
    