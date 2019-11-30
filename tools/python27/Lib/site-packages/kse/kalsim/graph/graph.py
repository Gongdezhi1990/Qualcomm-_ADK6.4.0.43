'''
Graph class
'''

import argparse
import logging
from functools import partial

from kats.framework.library.log import log_input, log_exception
from kats.library.graph.graph import Graph as GGraph, STREAM, ENDPOINT, OPERATOR, KOPERATOR, \
    STREAM_TYPE_SOURCE, STREAM_TYPE_SINK, GRAPH_FROM, GRAPH_TO
from kats.library.registry import get_instance

CALLBACK_CONSUME = 'callback_consume'
CALLBACK_EOF = 'callback_eof'


class Graph(object):
    '''
    Kalimba graph handler

    This class handles all elements in a class (streams, endpoints, operators and koperators).
    It has the capability to create, configure, connect, start streaming, stop streaming,
    disconnect and destroy.

    Args:
        stream (kats.kalsim.stream.stream_factory.StreamFactory): Stream factory
        endpoint (kats.kalimba.endpoint.endpoint_factory.EndpointFactory): Endpoint factory
        capability (kats.kalimba.capability.capability_factory.CapabilityFactory): Capability factry
        kcapability (kats.kalsim.capability.capability_factory.CapabilityFactory): KCapability factr
        kymera (kats.kalimba.kymera.kymera_base.KymeraBase): Kymera instance
        config (dict): Input data configuration
    '''

    def __init__(self, stream, endpoint, capability, kcapability, kymera, config):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        self._helper = argparse.Namespace()
        self._helper.stream = stream
        self._helper.endp = endpoint
        self._helper.cap = capability
        self._helper.kcap = kcapability
        self._helper.kymera = kymera

        self._config = config
        self._inst = argparse.Namespace()
        self._inst.st = []
        self._inst.ep = []
        self._inst.op = []
        self._inst.kop = []
        self._inst.conn = []
        self._graph = None

    @log_input(logging.INFO)
    def load(self, config):
        '''
        Reload configuration.

        This method is useful if configuration is not known when this helper is instantiated

        Args:
            config (dict): Input data configuration
        '''
        self._config = config

    @log_input(logging.INFO)
    def play(self, config=None):
        '''
        Play a graph.

        This will load and optional configuration data, and create, config, connect, start,
        wait for EOF, stop, disconnect and destroy a graph

        Args:
            config (dict): Input data configuration
        '''
        if config:
            self.load(config)
        self.create()
        self.config()
        self.connect()
        self.start()
        while self.check_active():
            import time
            time.sleep(0.5)
        self.stop()
        self.disconnect()
        self.destroy()

    @log_input(logging.INFO)
    @log_exception
    def create(self, **kwargs):
        '''
        Builds the internal graph from the configuration entries in input_data
        stream, endpoint, operator and graph.

        Creates all the elements (streams, endpoints and operators)

        Args:
            stream_smart (bool): Create streams only if they exist in the graph
            endpoint_smart (bool): Create endpoints only if they exist in the graph
            operator_smart (bool): Create operators only if they exist in the graph
        '''
        stream_smart = kwargs.pop('stream_smart', False)
        endpoint_smart = kwargs.pop('endpoint_smart', False)
        operator_smart = kwargs.pop('operator_smart', False)

        self._graph = GGraph(self._config)

        self._inst.st = [None] * self._graph.get_stream_num(None)
        for ind in range(self._graph.get_stream_num()):
            if not stream_smart or self._graph.check_node_in_graph(STREAM, ind):
                interface, stream_type, args, kwargs = self._graph.get_stream(None, ind)
                self._inst.st[ind] = self._helper.stream.get_instance(interface, stream_type,
                                                                      *args, **kwargs)
                self._log.info('creating stream%s interface:%s stream_type:%s', ind, interface,
                               stream_type)
                self._inst.st[ind].create()
            else:
                interface, stream_type, _, _ = self._graph.get_stream(None, ind)
                self._log.warning('omitting stream creation index:%s interface:%s stream_type:%s',
                                  ind, interface, stream_type)

        self._inst.ep = [None] * self._graph.get_endpoint_num()
        for ind in range(self._graph.get_endpoint_num()):
            if not endpoint_smart or self._graph.check_node_in_graph(ENDPOINT, ind):
                interface, endpoint_type, args, kwargs = self._graph.get_endpoint(None, ind)
                self._inst.ep[ind] = self._helper.endp.get_instance(interface, endpoint_type,
                                                                    *args, **kwargs)
                self._log.info('creating endpoint%s interface:%s endpoint_type:%s', ind, interface,
                               endpoint_type)
                self._inst.ep[ind].create()
            else:
                interface, endpoint_type, _, _ = self._graph.get_endpoint(None, ind)
                self._log.warning(
                    'omitting endpoint creation index:%s interface:%s endpoint_type:%s',
                    ind, interface, endpoint_type)

        self._inst.op = [None] * self._graph.get_operator_num()
        for ind in range(self._graph.get_operator_num()):
            if not operator_smart or self._graph.check_node_in_graph(OPERATOR, ind):
                interface, args, kwargs = self._graph.get_operator(ind)
                self._inst.op[ind] = self._helper.cap.get_instance(interface, *args, **kwargs)
                self._log.info('creating operator%s interface:%s', ind, interface)
                args, kwargs = self._graph.get_operator_method(ind, 'create')
                self._inst.op[ind].create(*args, **kwargs)
            else:
                interface, _, _ = self._graph.get_operator(ind)
                self._log.warning('omitting operator creation index:%s interface:%s', ind,
                                  interface)

        self._inst.kop = [None] * self._graph.get_koperator_num()
        for ind in range(self._graph.get_koperator_num()):
            if not operator_smart or self._graph.check_node_in_graph(KOPERATOR, ind):
                interface, args, kwargs = self._graph.get_koperator(ind)
                self._inst.kop[ind] = self._helper.kcap.get_instance(interface, *args, **kwargs)
                self._log.info('creating koperator%s interface:%s', ind, interface)
                self._inst.kop[ind].create()
            else:
                interface, _, _ = self._graph.get_koperator(ind)
                self._log.warning('omitting koperator creation index:%s interface:%s', ind,
                                  interface)

    @log_input(logging.INFO)
    @log_exception
    def config(self):
        '''
        Configures all elements (streams, endpoints and operators) in the already built internal
        graph.
        Synchronises endpoints as required
        '''
        for ind, stream in enumerate(self._inst.st):
            if stream is not None:
                kwargs = {}
                if stream.get_type() == STREAM_TYPE_SINK:
                    conns = self._graph.get_node_out_connections(STREAM, ind)
                    kwargs = {CALLBACK_CONSUME: [lambda *args, **kwargs: None] * 1}

                    for conn in conns:
                        source_terminal = 0
                        sink_type = conn[GRAPH_TO].get_type()
                        sink_number = conn[GRAPH_TO].get_index()
                        if sink_type == KOPERATOR:
                            sink_terminal = conn[GRAPH_TO].get_modifier(1)
                            consume = partial(self._inst.kop[sink_number].consume,
                                              input_num=sink_terminal)
                            kwargs[CALLBACK_CONSUME][source_terminal] = consume

                self._log.info('configuring stream%s', ind)
                stream.config(**kwargs)

        for ind, endpoint in enumerate(self._inst.ep):
            if endpoint is not None:
                self._log.info('configuring endpoint%s', ind)
                endpoint.config()

        for ind, operator in enumerate(self._inst.op):
            if operator is not None:
                self._log.info('configuring operator%s', ind)
                operator.config()

        for ind, operator in enumerate(self._inst.kop):
            if operator is not None:
                conns = self._graph.get_node_out_connections(KOPERATOR, ind)
                kwargs = {
                    CALLBACK_CONSUME: [lambda *args, **kwargs: None] * operator.output_num,
                    CALLBACK_EOF: [lambda *args, **kwargs: None] * operator.output_num
                }

                for conn in conns:
                    source_terminal = conn[GRAPH_FROM].get_modifier(1)
                    sink_type = conn[GRAPH_TO].get_type()
                    sink_number = conn[GRAPH_TO].get_index()
                    if sink_type == STREAM:
                        sink_terminal = 0
                        consume = partial(self._inst.st[sink_number].consume,
                                          input_num=sink_terminal)
                        kwargs[CALLBACK_CONSUME][source_terminal] = consume
                        eof = partial(self._inst.st[sink_number].eof_detected,
                                      input_num=sink_terminal)
                        kwargs[CALLBACK_EOF][source_terminal] = eof
                    elif sink_type == KOPERATOR:
                        sink_terminal = conn[GRAPH_TO].get_modifier(1)
                        consume = partial(self._inst.kop[sink_number].consume,
                                          input_num=sink_terminal)
                        kwargs[CALLBACK_CONSUME][source_terminal] = consume
                        eof = partial(self._inst.kop[sink_number].eof_detected,
                                      input_num=sink_terminal)
                        kwargs[CALLBACK_EOF][source_terminal] = eof
                    else:
                        raise RuntimeError('koperator%s connected to invalid node' % (ind))
                self._log.info('configuring koperator%s', ind)
                operator.config(**kwargs)

        for index in range(self._graph.get_sync_num()):
            syncs = self._graph.get_sync_connections(index)
            for ind, node in enumerate(syncs[:-1]):
                self._log.info('syncing %s to %s',
                               self._graph.get_sync(index)[ind],
                               self._graph.get_sync(index)[ind + 1])
                sid0 = self._inst.ep[node.get_index()].get_id()
                sid1 = self._inst.ep[syncs[ind + 1].get_index()].get_id()
                self._helper.kymera.stream_if_sync_sids(sid0, sid1)

    @log_input(logging.INFO)
    @log_exception
    def connect(self):
        '''
        Connect all elements (endpoints and operators) in the already built internal graph
        '''
        self._inst.conn = []
        for entry in self._graph.get_graph_connections():
            if (entry[GRAPH_FROM].get_type() != KOPERATOR and
                    entry[GRAPH_TO].get_type() != KOPERATOR and
                    entry[GRAPH_FROM].get_type() != STREAM and
                    entry[GRAPH_TO].get_type() != STREAM):
                num = entry[GRAPH_FROM].get_index()
                if entry[GRAPH_FROM].get_type() == ENDPOINT:
                    id1 = self._inst.ep[num].get_id()
                else:
                    num2 = entry[GRAPH_FROM].get_modifier(1)
                    id1 = self._inst.op[num].get_source_endpoint(num2)

                num = entry[GRAPH_TO].get_index()
                if entry[GRAPH_TO].get_type() == ENDPOINT:
                    id2 = self._inst.ep[num].get_id()
                else:
                    num2 = entry[GRAPH_TO].get_modifier(1)
                    id2 = self._inst.op[num].get_sink_endpoint(num2)

                self._log.info('connecting %s to %s', str(entry[GRAPH_FROM]), str(entry[GRAPH_TO]))
                self._inst.conn.append(self._helper.kymera.stream_if_connect(id1, id2))

    @log_input(logging.INFO)
    @log_exception
    def start(self):
        '''
        Starts streaming all source and sink streams in the already built internal
        graph
        '''
        # operators
        for operator in self._inst.op:
            if operator is not None:
                operator.start()

        kalcmd = get_instance('kalcmd')
        with kalcmd.get_lock_object():
            # sink streams
            for ind, stream in enumerate(self._inst.st):
                if stream is not None:
                    _, stream_type, _, _ = self._graph.get_stream(None, ind)
                    if stream_type == STREAM_TYPE_SINK:
                        stream.start()

            # source streams
            for ind, stream in enumerate(self._inst.st):
                if stream is not None:
                    _, stream_type, _, _ = self._graph.get_stream(None, ind)
                    if stream_type == STREAM_TYPE_SOURCE:
                        stream.start()

            # koperators
            for operator in self._inst.kop:
                if operator is not None:
                    operator.start()

    @log_exception
    def check_active(self):
        '''
        Checks if all source streams have reached EOF in the already built internal graph
        '''
        ret = False
        for ind, stream in enumerate(self._inst.st):
            if stream is not None:
                _, stream_type, _, _ = self._graph.get_stream(None, ind)
                if stream_type == STREAM_TYPE_SOURCE:
                    ret = ret or stream.check_active()
        return ret

    @log_input(logging.INFO)
    @log_exception
    def stop(self):
        '''
        Stops all elements (streams and operators) in the already built internal graph
        '''
        # kats operators
        for operator in self._inst.kop:
            if operator is not None:
                operator.stop()

        # source streams
        for ind, stream in enumerate(self._inst.st):
            if stream is not None:
                _, stream_type, _, _ = self._graph.get_stream(None, ind)
                if stream_type == STREAM_TYPE_SOURCE:
                    stream.stop()

        # sink streams
        for ind, stream in enumerate(self._inst.st):
            if stream is not None:
                _, stream_type, _, _ = self._graph.get_stream(None, ind)
                if stream_type == STREAM_TYPE_SINK:
                    stream.stop()

        # operators
        for operator in self._inst.op:
            if operator is not None:
                operator.stop()

    @log_input(logging.INFO)
    def disconnect(self):
        '''
        Disconnects all elements (endpoints and operators) in the already built graph
        '''
        self._helper.kymera.stream_if_transform_disconnect(self._inst.conn)

    @log_input(logging.INFO)
    def destroy(self):
        '''
        Destroy all streams, endpoints and operators in the already built internal graph
        '''
        for operator in self._inst.op:
            if operator is not None:
                operator.destroy()
                self._helper.cap.put_instance(operator)

        for operator in self._inst.kop:
            if operator is not None:
                operator.destroy()
                self._helper.kcap.put_instance(operator)

        for endpoint in self._inst.ep:
            if endpoint is not None:
                endpoint.destroy()
                self._helper.endp.put_instance(endpoint)

        for stream in self._inst.st:
            if stream is not None:
                stream.destroy()
                self._helper.stream.put_instance(stream)

        del self._graph
        self._inst.st = []
        self._inst.ep = []
        self._inst.op = []
        self._inst.kop = []

    @log_input(logging.INFO)
    def get_stream_num(self):
        '''
        Get number of stream instances available in the graph

        Returns:
            int: Number of stream instances
        '''
        return len(self._inst.st)

    @log_input(logging.INFO)
    def get_stream(self, index):
        '''
        Get stream instance

        Args:
            index (int): Zero based index of the stream

        Returns:
            kats.kalsim.stream.stream_base.StreamBase: Stream instance
        '''
        return self._inst.st[index]

    @log_input(logging.INFO)
    def get_endpoint_num(self):
        '''
        Get number of endpoint instances available in the graph

        Returns:
            int: Number of endpoint instances
        '''
        return len(self._inst.ep)

    @log_input(logging.INFO)
    def get_endpoint(self, index):
        '''
        Get endpoint instance

        Args:
            index (int): Zero based index of the endpoint

        Returns:
            kats.kalimba.endpoint.endpoint_base.EndpointBase: Endpoint instance
        '''
        return self._inst.ep[index]

    @log_input(logging.INFO)
    def get_operator_num(self):
        '''
        Get number of operator instances available in the graph

        Returns:
            int: Number of operator instances
        '''
        return len(self._inst.op)

    @log_input(logging.INFO)
    def get_operator(self, index):
        '''
        Get operator instance

        Args:
            index (int): Zero based index of the operator

        Returns:
            kats.kalimba.capability.capability_base.CapabilityBase: Operator instance
        '''
        return self._inst.op[index]

    @log_input(logging.INFO)
    def get_koperator_num(self):
        '''
        Get number of kats operator instances available in the graph

        Returns:
            int: Number of kats operator instances
        '''
        return len(self._inst.kop)

    @log_input(logging.INFO)
    def get_koperator(self, index):
        '''
        Get kats operator instance

        Args:
            index (int): Zero based index of the kats operator

        Returns:
            kats.kalsim.operator.operator_base.OperatorBase: Operator instance
        '''
        return self._inst.kop[index]
