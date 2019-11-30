# pylint: skip-file
#@PydevCodeAnalysisIgnore

import os
import time

if __name__ == '__main__':

    from kats.kalsim.stream.stream_base import CALLBACK_EOF

    INPUT_FILE_RAW = 'resource/153_Prompts_176.4_kHz_Music_Detected_8k.raw'
    OUTPUT_FILE_RAW = 'tmp/153_Prompts_176.4_kHz_Music_Detected_8k.raw'

    print('removing output files')
    try:
        os.remove(OUTPUT_FILE_RAW)
    except Exception:
        pass

    print('creating source stream')
    config = {
        'hydra_type': 'audio_slot',
        'hydra_bac_handle': 0,
        'hydra_audioslot': 0,
        'stream_flow_control_drive': 'kalsim',
        'stream_flow_control_block_size': 1,
        'stream_format': 16,
        'stream_backing': 'file',
        'stream_filename': INPUT_FILE_RAW,
        'stream_flow_control_rate': 8000,
    }
    st_source = stream.get_instance('hydra', 'source', **config)
    st_source.create()
    st_source.config()

    print('creating sink stream')
    config = {
        'hydra_type': 'audio_slot',
        'hydra_bac_handle': 0,
        'hydra_audioslot': 0,
        'stream_flow_control_drive': 'kalsim',
        'stream_flow_control_block_size': 1,
        'stream_format': 16,
        'stream_backing': 'file',
        'stream_filename': OUTPUT_FILE_RAW,
        'stream_flow_control_rate': 8000,
    }
    st_sink = stream.get_instance('hydra', 'sink', **config)
    st_sink.create()
    st_sink.config()

    print('creating endpoints')
    ep_source = kymera.stream_if_get_source('pcm', [0, 0])
    kymera.stream_if_configure_sid(ep_source, 'pcm_sample_rising_edge', 0)
    kymera.stream_if_configure_sid(ep_source, 'pcm_master_clock_rate', 512000)
    kymera.stream_if_configure_sid(ep_source, 'pcm_master_mode', 1)
    kymera.stream_if_configure_sid(ep_source, 'pcm_sample_format', 1)
    kymera.stream_if_configure_sid(ep_source, 'pcm_sync_rate', 8000)
    kymera.stream_if_configure_sid(ep_source, 'pcm_slot_count', 4)
    ep_sink = kymera.stream_if_get_sink('pcm', [0, 0])

    print('creating pass through operator')
    op = kymera.opmgr_create_operator(1)
    kymera.opmgr_operator_message(op, [10, 1])
    kymera.opmgr_operator_message(op, [11, 1])

    print('connecting endpoints')
    tr1 = kymera.stream_if_connect(ep_source, op | 0xa000)
    tr2 = kymera.stream_if_connect(op | 0x2000, ep_sink)

    print('start streaming')
    st_sink.start()
    kymera.opmgr_start_operators([op])
    st_source.start()

    for _ in range(12500):
        if not st_source.check_active():
            print('end of file detected')
            break
        time.sleep(0.005)
    else:
        raise RuntimeError('timeout streaming the file')

    print('stop streaming')
    st_source.stop()
    st_source.destroy()
    st_sink.stop()
    st_sink.destroy()

    print('release resources')
    kymera.opmgr_stop_operators([op])
    kymera.stream_if_transform_disconnect([tr1, tr2])
    kymera.opmgr_destroy_operators([op])

    kymera.stream_if_close_source(ep_source)
    kymera.stream_if_close_sink(ep_sink)
