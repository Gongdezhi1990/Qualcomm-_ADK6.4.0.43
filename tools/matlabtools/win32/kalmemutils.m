%KALMEMUTILS provides utiltiy functions to support KALREADVAL and KALWRITEVAL.
% Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd 
%   
% These utility functions are not intended to be used directly.

classdef kalmemutils

    properties(Constant)
        ERR_TWO_OFFSETS = 'You appear to have supplied two offsets, have you supplied a numeric and constant offset?';
        ERR_FORMAT = 'Format "%s" is not supported for this operation.';
        
        READ_SOURCE_REGISTER = 0;
        READ_SOURCE_PM       = 1;
        READ_SOURCE_DM       = 2;
        
        INPUT_TYPE_UNKNOWN  = 0;
        INPUT_TYPE_ADDRESS  = 1;
        INPUT_TYPE_REGISTER = 2;
        INPUT_TYPE_SYMBOL   = 3;
    end

    methods(Static, Access=private)
    
        function offset = checkNumericOffset(potentialOffset)
            if length(potentialOffset) ~= 1
                error('Please supply a scalar value as the offset.')
            end

            if potentialOffset ~= fix(potentialOffset)
                error('Please supply an integer as the offset.')
            end
            
            offset = potentialOffset;
        end
        
        function offset = resolveConstantSymbolOffset(offsetSymbol)
            % Retrieve info about all the constants
            temp_symb = kalvarprs('CONST');

            % Search for the constant
            temp = strcmpi(temp_symb, offsetSymbol);
            
            if sum(temp) == 0
                error('There was no match for the constant name "%s" that you supplied.', offsetSymbol)
            elseif sum(temp) ~= 1
                error('There is not a unique match for the constant name "%s" that you supplied.', offsetSymbol)
            end

            temp = (1:size(temp,1)) * temp(:,1);
            offset = temp_symb{temp,3};
            clear temp temp_symb
        end
        
    end % private static methods
    
    methods(Static, Access=public)
        
        % 'forbiddenFormats' is optional, and allows us to express differences in usage between reads and writes, as some
        % formats don't make sense, or are not supported, for both operations.
        function [offset, bankSel, numFormat, modeChanged] = parseExtraMemArgs(extraArgs, forbiddenFormats)
            
            % Defaults
            numFormat   = '';
            offset      = NaN;
            bankSel     = kalmemutils.READ_SOURCE_DM;
            modeChanged = false; % Tell the caller whether a change to the number format be persistent.
            
            % forbiddenFormats is not compulsory; explicitly ensure it's set to something.
            if nargin < 2
                forbiddenFormats = [];
            end
            forbiddenFormats = upper(forbiddenFormats);

            for i = 1 : length(extraArgs)
                if isnumeric(extraArgs{i})
                    if ~isnan(offset)
                        error(kalmemutils.ERR_TWO_OFFSETS);
                    end
                    offset = kalmemutils.checkNumericOffset(extraArgs{1});
                else
                    switch upper(extraArgs{i})
                        case 'PM'
                            bankSel = kalmemutils.READ_SOURCE_PM;
                        case {'FRAC', 'INT', 'INT16', 'INT24', 'UINT', 'HEX', 'ASCII', 'ASCIIPACK'}
                            if sum(ismember(forbiddenFormats, upper(extraArgs{i}))) ~= 0
                                error(kalmemutils.ERR_FORMAT, extraArgs{i})
                            end
                            numFormat = upper(extraArgs{i});
                        case {'MODEFRAC', 'MODEINT', 'MODEINT16', 'MODEINT24', 'MODEUINT', 'MODEHEX', 'MODEASCII', 'MODEASCIIPACK'}
                            numFormat = upper(extraArgs{i}(5:end));
                            if sum(ismember(forbiddenFormats, upper(numFormat))) ~= 0
                                error(kalmemutils.ERR_FORMAT, extraArgs{i})
                            end
                            modeChanged = true;
                        otherwise
                            % Arg was non-numeric and wasn't a recognised string, so it
                            % must be the name of a const to be used as an offset.
                            if ~isnan(offset)
                                error(kalmemutils.ERR_TWO_OFFSETS);
                            end
                            offset = kalmemutils.resolveConstantSymbolOffset(extraArgs{i});
                    end
                end
            end
            
        end % function
        
        function [inputType, addr] = parseNameArg(name, offset, bankSel, archInfo)
            
            if isnan(offset)
                offsetSpecified = false;
                offset = 0;
            else
                offsetSpecified = true;
            end
            
            ka        = kxs.instance();
            inputType = kalmemutils.INPUT_TYPE_UNKNOWN;
            
            if bankSel == kalmemutils.READ_SOURCE_PM
                addrsPerWord = kaldspinfo.addressesPerWordPM(archInfo);
            else
                addrsPerWord = kaldspinfo.addressesPerWordDM(archInfo);
            end
                
            % If 'name' is a string representation of a number, convert it to a number.
            if ~isnumeric(name) && ~isempty(str2num(name))
                name = str2num(name);
            end

            % Figure out what type of thing we've been asked to read (reg, symbol name
            % or memory address). If it's numeric, it's an address. Otherwise, we try
            % a register name, then lastly, treat it as a symbol and do a lookup.
            if isnumeric(name)
                inputType = kalmemutils.INPUT_TYPE_ADDRESS;
                
                addr = name(:)' + offset;

                % Check the address/addresses supplied are acceptable.
                if sum( (addr < 0) + (addr ~= fix(addr)) )
                    error('All memory addresses must be integers >= 0.')
                end

                if (offsetSpecified) && (length(addr) ~= 1)
                    error('Please specify an offset or a range of values, not both.')
                end
                
                if sum(mod(addr, addrsPerWord)) ~= 0
                    error('Memory accesses must be to word-aligned addresses on octet-addressable chips.')
                end
                
            elseif ka.registerNameMap.isKey(upper(name))
                inputType = kalmemutils.INPUT_TYPE_REGISTER;
                addr = ka.registerNameMap(upper(name));
                
            else
                inputType = kalmemutils.INPUT_TYPE_SYMBOL;

                % First search for a match with the pattern supplied
                if bankSel == kalmemutils.READ_SOURCE_PM
                    symb = [kalsymfind(name, 'PM')];
                else
                    symb = [kalsymfind(name); kalsymfind(name, 'CONST')];
                end
                
                % cope with combined constants and symbols in ELF files
                if (size(symb, 1) == 2) && (strcmpi(symb{1,1},symb{2,1}))
                    tmp{1,1} = symb{1,1};
                    tmp{1,2} = symb{1,2};
                    tmp{1,3} = symb{1,3};
                    symb = tmp;
                end

                % did we get too many results?
                if size(symb, 1) == 0
                    error('There is no match for the name you have supplied.')
                elseif size(symb, 1) > 1
                    namesThatMatch = strjoin(symb(:,1)', '\n  ');
                    error('There are too many matches for the name you have supplied. They were:\n  %s', namesThatMatch)
                end

                % Calculate the address or addresses that we should read. 
                addr            = symb{3};
                sizeInAddresses = symb{2};
                
                % Disallow out-of-bounds accesses via symbols.
                if (offsetSpecified && offset >= sizeInAddresses)
                    error('The offset supplied is larger than the variable')
                end
                
                % If not using an offset, generate a range from addr, sizeInAddresses long. 
                % This is surprisingly easy to get wrong.
                % Example time, consider a one-word variable at address "4".
                % a) Word addressable memory. Then sizeInAddresses == 1. We generate addr = 4 to 4 = 4.
                % b) Octet addressable memory. Then sizeInAddresses == 4. We generate addr = 4,5,6,7.
                % Otherwise, if an offset was specified, just produce a single address offset by that amount.
                % Note that this means that we only return the first word or octet of multi-unit "structure"
                % fields, which is potentially confusing.
                % Note that the units of the offset are the same units as sizeInAddresses, i.e. the smallest addressable unit.
                if offsetSpecified
                    addr = addr + offset;
                else
                    addr = (addr:(addr + sizeInAddresses - 1));
                    % If we're on a Kalimba that supports octet addressing, then remove
                    % addresses that are not the start of words, i.e. convert
                    % [4,5,6,7,8,9,10,11] into [4,8].
                    addr = addr(1:addrsPerWord:end);
                end

            end
        
        end % function
        
    end % public static methods
    
end % class
