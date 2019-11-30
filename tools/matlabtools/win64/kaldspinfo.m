%KALDSPINFO returns fixed architectural properties of Kalimba processors.
% Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd 
%   
% These utility functions are not intended to be used directly.

classdef kaldspinfo
    
    methods(Static, Access=private)
    
        function lazyLoad()
            persistent dllLoaded;
            if isempty(dllLoaded)
                if ~libisloaded('dspinfo')
                    loadlibrary('dspinfo', @dspinfo_proto);
                end
                dllLoaded = true;
            end
        end
        
    end
    
    methods(Static, Access=public)
    
        function archInfo = archInfoFromArchNumber(archNumber)
            kaldspinfo.lazyLoad();
            
            if archNumber < 1 || archNumber > 5
                error('Unsupported Kalimba architecture number: %d", archNumber');
            end
            
            archNumberForC = int32(archNumber);
            archInfoPtr = calllib('dspinfo', 'kalarchinfo_from_arch', archNumberForC);
            archInfo = archInfoPtr.value;
        end
        
        function n = addressesPerWordPM(archInfo)
            if archInfo.pm_octet_addressing == 0
                n = 1;
            else
                n = archInfo.pm_data_width / 8;
            end
        end
        
        function n = addressesPerWordDM(archInfo)
            if archInfo.dm_octet_addressing == 0
                n = 1;
            else
                n = archInfo.dm_data_width / 8;
            end
        end
    end
end
