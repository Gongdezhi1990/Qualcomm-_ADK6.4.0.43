function [z q] = sbc_tables(arg1)

% input 1, table type
%
%
%Tables:
%
% CONST symbol name             field offset                ram symbol name                 size    type
%   enc/dec					
%$sbc.win_coefs_m4              WIN_COEFS_M4_FIELD          $sbc.win_coefs_m4_ram           40      frac
%$sbc.win_coefs_m8              WIN_COEFS_M8_FIELD          $sbc.win_coefs_m8_ram           80      frac
%   encoder					
%$sbcenc.analysis_coefs_m4      ANALYSIS_COEFS_M4_FIELD     $sbcenc.analysis_coefs_m4_ram   32      frac
%$sbcenc.analysis_coefs_m8      ANALYSIS_COEFS_M8_FIELD     $sbcenc.analysis_coefs_m8_ram	128     frac
%$sbcenc.level_coefs            LEVEL_COEFS_FIELD           $sbcenc.level_coefs_ram         15      frac
%   decoder					
%$sbcdec.synthesis_coefs_m4     SYNTHESIS_COEFS_M4_FIELD    $sbcdec.synthesis_coefs_m4_ram	32      frac
%$sbcdec.synthesis_coefs_m8     SYNTHESIS_COEFS_M8_FIELD,   $sbcdec.synthesis_coefs_m8_ram	128     frac
%$sbcdec.levelrecip_coefs       LEVELRECIP_COEFS_FIELD      $sbcdec.levelrecip_coefs_ram	15      frac
%$sbcdec.bitmask_lookup         BITMASK_LOOKUP_FIELD        $sbcdec.bitmask_lookup_ram      17      int
				


if strcmp(arg1, 'ana_m4')
    z = gen_analysis_m4();
elseif strcmp(arg1, 'ana_m8')
    z = gen_analysis_m8();
elseif strcmp(arg1, 'lev_rcp')
    z = gen_level_recip_coeffs();
elseif strcmp(arg1, 'synth_m4')
    z = gen_synth_m4();
elseif strcmp(arg1, 'synth_m8')
    z = gen_synth_m8();
elseif strcmp(arg1, 'bit')
    z = gen_bitmask();
elseif strcmp(arg1, 'lev')
    z = gen_lev_coeffs();
elseif strcmp(arg1, 'm4')
    z = gen_m4_coeffs();
elseif strcmp(arg1, 'm8')
    z = gen_m8_coeffs();
else
    error('invalid argument');
end

q = [];
%if strcmp(tp, 'int');
%    q = pack_int_32(z);
%elseif strcmp(tp, 'q31');
%    q = pack_Q_32(z);
%end
%p = reshape(t,8,8)
%dlmwrite('c:\loc\coef.dat', p, 'precision', '0x%.4x, ', 'delimiter','');

end

%
% table generation functions
% Warning - recent code uses 16 words, see B-134624

function z = gen_lev_coeffs()
    z = zeros(15, 1);
    for k=2:16
        z(k-1) = (2^(k)-1)*(2^7);
    end
end

function z = gen_bitmask()
    z = zeros(17, 1);
    for k=0:16
        z(k+1) = 2^(k)-1;
    end
end


function z = gen_synth_m8()
    z = zeros(128, 1);
    for k=0:15
        for i=0:7
            z(1 + k*8 + i) = cos((i+.5)*(k+4)*(pi/8));
        end
    end
end

function z = gen_synth_m4()
    z = zeros(32, 1);
    for k=0:7
        for i=0:3
            z(1 + k*4 + i) = cos((i+.5)*(k+2)*(pi/4));
        end
    end
end

function z = gen_level_recip_coeffs()
    z = zeros(15, 1);
    for i=2:16
        z(i-1) = (2.^(i)) ./ (2.^(i)-1);
    end
end


function z = gen_analysis_m8()
    z = zeros(128, 1);
    for i=0:7
        for k=0:15
            z(1 + k + 16*i) = cos((i+.5)*(k-4)*(pi/8));
        end
    end
end

function z = gen_analysis_m4()
    z = zeros(32, 1);
    for i=0:3
        for k=0:7
            z(1 + k + 8*i) = cos((i+.5)*(k-2)*(pi/4));
        end
    end
end

function z = gen_m4_coeffs()
    z = 2 * [0.000000000000000,  0.000536548976000,  0.001491883570000,  0.002733709040000,...  
    0.003837201930000,  0.003892051490000,  0.001865816910000,  -0.003060122860000,...  
    0.010913762000000,  0.020438508700000,  0.028875739200000,  0.032193929000000,...  
    0.025876781100000,  0.006132451860000,  -0.028821727400000,  -0.077646349400000,...  
    0.135593274000000,  0.194987841000000,  0.246636662000000,  0.281828203000000,...  
    0.294315332000000,  0.281828203000000,  0.246636662000000,  0.194987841000000,...  
    -0.135593274000000,  -0.077646349400000,  -0.028821727400000,  0.006132451860000,...  
    0.025876781100000,  0.032193929000000,  0.028875739200000,  0.020438508700000,...  
    -0.010913762000000,  -0.003060122860000,  0.001865816910000,  0.003892051490000,...  
    0.003837201930000,  0.002733709040000,  0.001491883570000,  0.000536548976000];  
end
function z = gen_m8_coeffs()
    %z = csvread('c:\loc\sbc_m8.dat');
    %z = z*4;
    z = 4 * [0.000000000000000,  0.000156575398000,  0.000343256425000,  0.000554620202000,...  
    0.000823919506000,  0.001139925070000,  0.001476401690000,  0.001783717250000,...  
    0.002011825420000,  0.002103719890000,  0.001994545540000,  0.001616562830000,...  
    0.000902154502000,  -0.000178805361000,  -0.001649730980000,  -0.003497174540000,...  
    0.005659494730000,  0.008029411630000,  0.010458444300000,  0.012747233500000,...  
    0.014652526300000,  0.015904560300000,  0.016220847100000,  0.015318410600000,...  
    0.012937180600000,  0.008857575400000,  0.002924084420000,  -0.004915780240000,...  
    -0.014640407600000,  -0.026109875200000,  -0.039075138100000,  -0.053187303200000,...  
    0.067998943100000,  0.082984757800000,  0.097575391800000,  0.111196689000000,... 
    0.123264548000000,  0.133264415000000,  0.140753505000000,  0.145389847000000,... 
    0.146955068000000,  0.145389847000000,  0.140753505000000,  0.133264415000000,...  
    0.123264548000000,  0.111196689000000,  0.097575391800000,  0.082984757800000,...  
    -0.067998943100000,  -0.053187303200000,  -0.039075138100000,  -0.026109875200000,...  
    -0.014640407600000,  -0.004915780240000,  0.002924084420000,  0.008857575400000,...  
    0.012937180600000,  0.015318410600000,  0.016220847100000,  0.015904560300000,...  
    0.014652526300000,  0.012747233500000,  0.010458444300000,  0.008029411630000,...  
    -0.005659494730000,  -0.003497174540000,  -0.001649730980000,  -0.000178805361000,...  
    0.000902154502000,  0.001616562830000,  0.001994545540000,  0.002103719890000,...  
    0.002011825420000,  0.001783717250000,  0.001476401690000,  0.001139925070000,...  
    0.000823919506000,  0.000554620202000,  0.000343256425000,  0.000156575398000];
end




% Packing functions
%

% pack: one q31 value into one 32-bit unsigned int
function q = pack_Q_32(x)
    M = length(x);
    q = zeros(M, 1);
    for i=1:M    
        x_I = floor(x(i)*2^31);
        if x_I == 2^31;
            x_I = 2^31 - 1;
        end
        if x_I < 0
            x_I = x_I + 2^32;
        end
        q(i) = x_I;
    end
end

% pack q31 value into two 16-bit unsigned ints
function q = pack_Q_16(x)

    M = length(x);
    q = zeros(2*M, 1);
    for i=1:M
        x_I = floor(x(i)*2^31);
        if x_I == 2^31;
            x_I = 2^31 - 1;
        end
        if x_I < 0
            x_I = x_I + 2^32;
        end
        x_H = floor(x_I / 65536);
        x_L = x_I - (x_H * 65536);
        q(2*i-1) = x_H;
        q(2*i) = x_L;
    end
end

% pack one signed int32 value into two 16-bit unsigned int
function q = pack_uint_16_int(x)

    M = length(x);
    q = zeros(1, 2*M);
    for i=1:M
        x_I = x(i);
        if x_I == 2^31;
            x_I = 2^31 - 1;
        end
        if x_I < 0
            x_I = x_I + 2^32;
        end
        x_H = floor(x_I / 65536);
        x_L = x_I - (x_H * 65536);
        q(2*i-1) = x_H;
        q(2*i) = x_L;
    end
end


% pack: one signed int32 value into one 32-bit unsigned int
function q = pack_int_32(x)
    M = length(x);
    q = zeros(M, 1);
    for i=1:M    
        x_I = floor(x(i));
        if x_I == 2^31;
            x_I = 2^31 - 1;
        end
        if x_I < 0
            x_I = x_I + 2^32;
        end
        q(i) = x_I;
    end
end

% debug functions
%


function z = get_dsp_vars()
% get RAM tables from DSP memory (static SBC variant only!)    

tab_m4 = kalreadval('$sbc.win_coefs_m4_ram', 'frac');
tab_m8 = kalreadval('$sbc.win_coefs_m8_ram', 'frac');

tab_a_m4 = kalreadval('$sbcenc.analysis_coefs_m4_ram', 'frac');
tab_a_m8 = kalreadval('$sbcenc.analysis_coefs_m8_ram', 'frac');
tab_lev = kalreadval('$sbcenc.level_coefs_ram', 'int');

tab_s_m4 = kalreadval('$sbcdec.synthesis_coefs_m4_ram', 'frac');
tab_s_m8 = kalreadval('$sbcdec.synthesis_coefs_m8_ram', 'frac');
tab_levrcp = kalreadval('$sbcdec.levelrecip_coefs_ram', 'frac');
tab_bitmask = kalreadval('$sbcdec.bitmask_lookup_ram', 'int');



end
