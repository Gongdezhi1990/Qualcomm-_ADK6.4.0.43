function [ varargout ] = kalsymfind( srch_str, memRegion )
%KALSYMFIND searches through the symbol table for matching symbol names.
% Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd 
% 
%   KALSYMFIND('SRCH_STR') searches for symbols in data memory matching the
%   search string SRCH_STR outputing the results to the command window. Two
%   wildcards are allowed '*' and '?', where '*' means zero to any number of any
%   characters acceptable and '?' means any one character.
%
%   KALSYMFIND('SRCH_STR', 'GROUP') limits matches to the specified group, if
%   omitted (as above KALSYMFIND defaults to DM).
%   Available groups are:
%      DM     - variables from data memory, returns variable name, size and start
%               address data memory
%      PM     - labels from program memory, returns label name, line number in
%               source code file and address in program memory
%      CONST  - symbols from constants, returns constant name, size (always 1)
%               and value
%      MODULE - symbols from program memory, returns one entry for each line of
%               each matching module, containg, module name, line number from the
%               corresponding source code file and location in program code.
%
%   [SYM] = KALSYMFIND('SRCH_STR') returns the results in a cell array where:
%                 SYM{i,1}    SYM{i,2}    SYM{i,3}
%      DM          name        size       address
%
%      PM          name     line number    value
%
%      CONST       name      size (1)     address
%
%      MODULE      name     line number   address
%
%   See also kalloadsym, kalreadval, kalwriteval, kalrunning.
%

if nargin == 0
    % No inputs => display help.
    help kalsymfind
    return
elseif nargin == 1
    % Assume a search string has been entered, and default to looking for symbols in DM.
    memRegion = 'DM';
end

memRegion = upper(memRegion);
if ~ischar(memRegion) || ~( strcmp(memRegion, 'DM') || strcmp(memRegion, 'PM') || strcmp(memRegion, 'CONST') || strcmp(memRegion, 'MODULE') )
    error('Invalid value for argument GROUP. See help kalsymfind for valid values.');
end

% try and work out if we are matching the name or the address
addrSearch = 0;
if ~isstr(srch_str)
    % its a number
    addrSearch = 1;
    addr = srch_str;
elseif length(srch_str) > 2
    if '0x' == srch_str(1:2)
        % its a value in hex
        addrSearch = 1;
        addr = hex2dec(srch_str(3:end));
    end
elseif ~isempty(str2num(srch_str))
    % its a value
    addrSearch = 1;
    addr = str2num(srch_str);
end

% load the symbols
symbols = kalvarprs(memRegion);
dspinfo = kalvarprs('DSPINFO');
srch_str_modified_flag = 0;

% have we been given an address to search for or a name
if addrSearch

    % build arrays of start and end addresses
    startAddr = cell2mat(symbols(:,3));
    endAddr   = startAddr + cell2mat(symbols(:,2));

    
    % convert -ve addresses into their unsigned equivalent
    if addr < 0
        addr = addr + 2^24;
    end
    
    symb_ind = and( (startAddr <= addr), (endAddr > addr) );

else

    % get the list of symbol names
    symb = lower(strvcat( symbols{:,1} ));
    symb_ind = [1:size(symb,1)]';

    % Set the search string length
    srch_str_len = length( srch_str );

    % Convert to lower case
    srch_str = lower(srch_str);

    % Work through the search string
    for i = 1:(srch_str_len-1)

        % Determine what character we are on
        switch srch_str(i)
         case '?'
          % '?' character, remove one column from symbols
          if isempty(symb), break; end

          symb = symb(:,2:end);

         case '*'
          % '*' character, search entire database for next search character
          if isempty(symb), break; end

          [res_row res_col] = find(symb == srch_str(i+1));

          % Initiate the temporary store
          temp = [];

          for ii = 1:length(res_row)
              % Work through the results
              temp = strvcat(temp, symb(res_row(ii),res_col(ii):end));
          end

          % Store the results
          symb = temp;
          symb_ind = symb_ind(res_row);

         otherwise
          % Search character, not a wild card
          if isempty(symb), break; end
          res = find(symb(:,1) == srch_str(i));

          symb = symb(res,2:end);
          symb_ind = symb_ind(res);

        end
    end

    if (~isempty(symb))
        % Check what the last character is
        switch srch_str(end)
         case '?'
          symb = symb(:,2:end);
          if size(symb,2) ~= 0
              res = find(symb(:,1) == ' ');
              symb_ind = symb_ind(res);
          end

         case '*'
          % All the results found are valid

         otherwise
          % Search character, not a wild card
          res = find(symb(:,1) == srch_str(end));

          symb = symb(res,2:end);
          symb_ind = symb_ind(res);

          if size(symb,2) ~= 0
              res = find(symb(:,1) == ' ');
              symb_ind = symb_ind(res);
          end
        end
    end
    symb_ind = unique( symb_ind );
end

% reduce the results down
clear res
res = symbols(symb_ind,:);
if srch_str_modified_flag && isempty(res)
   srch_str = ['$' srch_str(4:end)];
   res = kalsymfind(srch_str, memRegion);
   symb_ind = 1:size(res, 1);
end

if nargout == 0
    % Store spaces & dashes to make the output pretty
    dashes = '  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  ';

    % Put a string in for constants
    if strcmpi(memRegion,'const')
        valQual = 'Value   - ';
    else
        valQual = 'Address - ';
    end

    % Put a space in
    fprintf('\n')

    if isempty(res)
        fprintf('   No matching symbols have been found\n')
    end

    for s = res'

        % combine the name and size
        if (strcmpi(memRegion, 'pm') || strcmpi(memRegion, 'const'))
            nameAndSize = s{1};
        else
            nameAndSize = [s{1} '[' int2str(s{2}) ']'];
        end
        
        % word out display width of data
        if (strcmpi(memRegion, 'pm') || strcmpi(memRegion, 'dm') || strcmpi(memRegion, 'module'))
            value_hex_width = dspinfo.addr_width/4;
        else
            value_hex_width = dspinfo.data_width/4;
        end
        
        % make up the name, size and dashes
        nameDisp = [nameAndSize dashes(length(nameAndSize):end)];
        % calculate the UNIT value to make hex
        hexValue = (s{3}<0)*2^dspinfo.data_width + s{3};

        fprintf('   %s%s&0x%0*X - %-d\n', nameDisp, valQual, value_hex_width, hexValue, s{3});
    end

    % Put a space in
    fprintf('\n');
else
    % supply the results
    varargout{1} = res;
end
