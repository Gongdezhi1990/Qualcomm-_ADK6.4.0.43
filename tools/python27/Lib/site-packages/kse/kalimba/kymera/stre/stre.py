'''
Stre Kymera handler
'''

import logging

from kats.framework.library.docstring import inherit_docstring
from ..generic.accmd import Accmd
from ..kymera_base import KymeraBase


class KymeraStre(KymeraBase):
    '''
    Kymera interface for stre

    This class is just an adapter for the kymera api to the accmd api

    Args:
        uut (kats.kalimba.uut.uut_base.UutBase): Uut instance
    '''

    platform = 'stre'
    interface = 'accmd'

    def __init__(self, uut, *args, **kwargs):
        inherit_docstring(self)

        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        self._accmd = Accmd(uut, *args, **kwargs)
        super(KymeraStre, self).__init__(uut, *args, **kwargs)

    def opmgr_create_operator(self, cap_id):
        return self._accmd.create_operator(cap_id)

    def opmgr_create_operator_ex(self, cap_id, priority=0, processor_id=0):
        return self._accmd.create_operator_ex(cap_id,
                                              priority=priority,
                                              processor_id=processor_id)

    def opmgr_destroy_operators(self, operators):
        self._accmd.destroy_operators(operators)

    def opmgr_start_operators(self, operators):
        self._accmd.start_operators(operators)

    def opmgr_stop_operators(self, operators):
        self._accmd.stop_operators(operators)

    def opmgr_reset_operators(self, operators):
        self._accmd.reset_operators(operators)

    def opmgr_operator_message(self, op_id, msg):
        return self._accmd.operator_message(op_id, msg)

    def opmgr_get_capid_list(self):
        cap_list = []
        num, cap = self._accmd.get_capabilities(0)
        while len(cap_list) < num and cap != []:
            if len(cap_list):
                _, cap = self._accmd.get_capabilities(len(cap_list))
            cap_list += cap
        return cap_list

    def opmgr_get_opid_list(self, cap_id=0):
        op_list = []
        num, op_ids = self._accmd.get_operators(cap_id, 0)
        while len(op_list) < num and op_ids:
            if len(op_list):  # do not re-read in the first iteration (already done)
                _, op_ids = self._accmd.get_operators(cap_id, len(op_list))
            op_list += op_ids
        return op_list

    def opmgr_get_cap_info(self, cap_id):
        _ = cap_id
        raise NotImplementedError('opmgr_get_cap_info not implemented')

    def stream_if_get_source(self, endpoint_type, params):
        return self._accmd.stream_get_source(endpoint_type, *params)

    def stream_if_get_sink(self, endpoint_type, params):
        return self._accmd.stream_get_sink(endpoint_type, *params)

    def stream_if_close_source(self, source_id):
        self._accmd.stream_close_source(source_id)

    def stream_if_close_sink(self, sink_id):
        self._accmd.stream_close_sink(sink_id)

    def stream_if_configure_sid(self, ep_id, key, value):
        self._accmd.stream_configure(ep_id, key, value)

    def stream_if_connect(self, source_id, sink_id):
        return self._accmd.stream_connect(source_id, sink_id)

    def stream_if_transform_disconnect(self, transforms):
        for transform in transforms:
            self._accmd.stream_transform_disconnect(transform)

    def stream_if_sync_sids(self, ep1, ep2):
        self._accmd.stream_sync_sid(ep1, ep2)

    def stream_get_connection_list(self, source_id, sink_id):
        _ = source_id
        _ = sink_id
        raise NotImplementedError('stream_get_connection_list not implemented')
