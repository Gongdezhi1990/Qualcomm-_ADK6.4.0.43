function [hO, nameO, pcO] = kalpcprofiler(T, da1, da2)
%KALPCPROFILER profile Kalimba DSP code by polling the program counter (PC)
% Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd 
% 
%   KALPCPROFILER profiles PC for 10 seconds and prints collected
%   information.
%
%   KALPCPROFILER(T) profiles PC for T seconds and prints collected
%   information. T must be smaller than 120.
%
%   [H, M, PB] = KALPCPROFILER(T) returns the profile data. H is a vector
%   containing the normalized histogram of PC, after PC values have been
%   matched to modules. Corresponding module names are given in M. PB
%   contains the polled PC values.
%
%   KALPCPROFILER(T, MODE) displays the information in the form
%   specified in MODE. MODE takes the following values:
%
%   'print'       : displays the information as text on screen
%   'bar'         : displays the information using horizontal bars
%   'pie'         : displays the information in a pie chart
%   'cat'         : categorizes the results into "libraries" (see NOTE 2)
%   'application' : displays the results for "application" modules (see NOTE 2)
%   otherwise : only displays the information for modules that contain this
%               text
%
%   any other value for MODE will print the information.
%
%   [...] = KALPCPROFILER(T, MODE, N) displays the information only for the
%   N - 1 most frequent modules and all the rest of the modules will be
%   grouped together as 'other'.
%
%   [...] = KALPCPROFILER(T, N) displays/returns the information only for
%   the N - 1 most frequent modules and all the rest of the modules will be
%   grouped together as 'other'.
%
%   NOTE 1: When using 'bar' or 'pie' charts it is advisable to limit the
%   number of modules displayed in order to get a readable graph.
%
%
%   NOTE 2: Categorizing mode, groups the modules with the same category,
%   together. Category of a module is the text before the second dot (.) in
%   its name, excluding possible "flash.code " at the begining. if there is
%   only one dot (.) in the module name, the module is categorized as
%   "Application".
%
%   See also kalmodname, kalreadval, kalwriteval, kalsymfind,
%

% Default profiling time to 10 seconds if not otherwise specified.
if nargin < 1
   T = 10;
end

% Show output if we're not assigning the results to anything.
displayOn = (nargout < 1);

% Initialize module boundaries
allModuleEntries = kalvarprs('MODULE');
moduleNames      = unique(allModuleEntries(:, 1));
moduleCount      = length(moduleNames);
moduleStarts     = zeros(1, moduleCount);
moduleEnds       = zeros(1, moduleCount);

currentEntry = 1;
% Add a dummy entry to the end, to ensure termination of the 'while' below.
allModuleEntries{end + 1, 1} = [];

for i = 1:moduleCount
    % This reassignment looks odd, but the unique() above does not preserve order.
    moduleNames{i}  = allModuleEntries{currentEntry, 1};
    moduleStarts(i) = allModuleEntries{currentEntry, 3};
   
    % Skip to the next module
    currentEntry = currentEntry + 1;
    while isequal(allModuleEntries{currentEntry, 1}, moduleNames{i})
        currentEntry = currentEntry + 1;
    end

    moduleEnds(i) = allModuleEntries{currentEntry - 1, 3};
end

if displayOn
   fprintf('Profiling for %1.1f seconds ...', T)
end

% Read the PC via hal_pc_profile, in chunks of 1000, until the desired clock time has elapsed.
ka = kxs.instance();
pcSamples = [];
tic;          % Start the Matlab stopwatch
while toc < T % Check the Matlab stopwatch
    pcSamplesChunk = ka.pcprofile(1000);
    pcSamples      = [pcSamples, pcSamplesChunk];
end

moduleHits = zeros(1, moduleCount);
for i = 1:moduleCount
    % Record a one when all pc values are within the bounds of the module.
    moduleHits(i) = sum(pcSamples >= moduleStarts(i) & pcSamples <= moduleEnds(i));
end

% The total number of module hits should equal the number of samples, unless the PC has 
% gone off to some unknown code.
if sum(moduleHits) < length(pcSamples)
    warning('Some of the sampled PC values were in unknown code.')
end

% Sort the results; reorder the names using the returned indices from the sort.
[moduleHits, I] = sort(moduleHits, 'descend');
moduleNames = moduleNames(I);

L    = moduleCount;
name = moduleNames;
h    = moduleHits;
pc   = pcSamples;

% if the second input argument is a text other than bar, pie or print, then
% put the modules containing that text first
filter_index = 0;
if nargin > 1 && ischar(da1) && ~isequal(lower(da1), 'bar') ...
      && ~isequal(lower(da1), 'pie') && ~isequal(lower(da1), 'print') ...
      && ~isequal(lower(da1), 'cat')
   I = nan(size(h));
   st = 1;
   if ~isequal(lower(da1), 'application')
      for i = 1:length(I)
         if ~isempty(strfind(lower(name{i}), da1))
            I(i) = st;
            filter_index = st;
            st = st + 1;
         end
      end
   else
      for i = 1:length(I)
         name_test = name{i};
         if length(name_test)>12 && isequal(name_test(1:11), 'flash.code ')
            name_test = name_test(12:end);
         end
         if length(strfind(name_test, '.'))<2
            I(i) = st;
            filter_index = st;
            st = st + 1;
         end
      end
   end
   if ~exist('da2', 'var')
      da2 = st;
   end
   for i = 1:length(I)
      if isnan(I(i))
         I(i) = st;
         st = st + 1;
      end
   end
   h(I) = h;
   name(I) = name;
   filter_text = da1;
   da1 = 'print';
elseif nargin > 1 && isequal(lower(da1), 'cat')
   % If cat keyword is selected categorize modules into "libraries"
   category = {};
   h_category = [];
   for i = 1:length(I)
      curr_name = name{i};
      if length(curr_name)>12 && isequal(curr_name(1:11), 'flash.code ')
         curr_name = curr_name(12:end);
      end
      k = strfind(curr_name, '.');
      if length(k)>=2
         curr_category = curr_name(1:k(2)-1);
      else
         curr_category = 'Application';
      end
      I = ismember(category, curr_category);
      if sum(I)==0
         category{end + 1} = curr_category;
         I = length(category);
         h_category(I) = 0;
      end
      h_category(I) = h_category(I) + h(i);
   end
   h = h_category;
   name = category;
   L = length(h);

   % sort the results
   [h, I] = sort(h, 'descend');
   name = name(I);
end

if displayOn
   fprintf('%d samples collected. \n', length(pc))
   % prepare for display
   if nargin<3 && ~exist('da2', 'var')
      if nargin<2
         da1 = 'print';
      end
      if ischar(da1)
         da2 = L;
      else
         da2 = da1;
         da1 = 'print';
      end
   end
   N = min(da2, L);

   x = [h(1:N-1) sum(h(N:end))];
   x = x./sum(x);
   y = name(1:N);
   if N<L,   y{end} = 'other'; end
   % if filtered the results using text
   if filter_index>=N
      x(end) = sum(h(N:filter_index))/sum(h);
      y{end} = ['other ' filter_text];
      y{end + 1} = ['other non-' filter_text];
      x(end + 1) = sum(h(filter_index+1:end))/sum(h);
      N = N + 1;
   end

   % Display the values
   switch lower(da1)
      case('bar')
         barh(x)
         set(gca, 'YTick', 1:N)
         set(gca, 'YTickLabel', y)
      case('pie')
         pie(x, y)
      otherwise
         for i = 1:N
            fprintf('  %-70s%6.2f%%\n', y{i}, x(i)*100 );
         end
   end
end

if nargout>0
   hO = h;
   nameO = name;
   pcO = pc;
end
