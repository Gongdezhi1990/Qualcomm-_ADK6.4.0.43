'''
Graph library
'''

import copy
import logging
from itertools import groupby

from kats.framework.library.schema import DefaultValidatingDraft4Validator

STREAM = 'stream'
STREAM_TYPE = 'type'
STREAM_TYPE_SOURCE = 'source'  # TODO this has to be synchronised with stream base class
STREAM_TYPE_SINK = 'sink'  # TODO this has to be synchronised with stream base class
STREAM_INTERFACE = 'interface'
STREAM_ARGS = 'args'
STREAM_KWARGS = 'kwargs'

ENDPOINT = 'endpoint'
ENDPOINT_TYPE = 'type'
ENDPOINT_TYPE_SOURCE = 'source'  # TODO this has to be synchronised with endpoint base class
ENDPOINT_TYPE_SINK = 'sink'  # TODO this has to be synchronised with endpoint base class
ENDPOINT_INTERFACE = 'interface'
ENDPOINT_ARGS = 'args'
ENDPOINT_KWARGS = 'kwargs'

OPERATOR = 'operator'
OPERATOR_INTERFACE = 'interface'
OPERATOR_ARGS = 'args'
OPERATOR_KWARGS = 'kwargs'
OPERATOR_CREATE = 'create'

KOPERATOR = 'koperator'
KOPERATOR_INTERFACE = 'interface'
KOPERATOR_ARGS = 'args'
KOPERATOR_KWARGS = 'kwargs'

GRAPH = 'graph'
SYNC = 'sync'

INPUT_DATA_SCHEMA = {
    'type': 'object',
    'default': {},
    'properties': {
        STREAM: {
            'type': 'array',
            'default': [],
            'items': {
                'type': 'object',
                'required': [STREAM_TYPE, STREAM_INTERFACE],
                'properties': {
                    STREAM_TYPE: {
                        'type': 'string',
                        'enum': [STREAM_TYPE_SOURCE, STREAM_TYPE_SINK],
                    },
                    STREAM_INTERFACE: {
                        'type': 'string',
                    },
                    STREAM_ARGS: {
                        'type': 'array',
                        'default': []
                    },
                    STREAM_KWARGS: {
                        'type': 'object',
                        'default': {}
                    },
                }
            }
        },
        ENDPOINT: {
            'type': 'array',
            'uniqueItems': True,
            'default': [],
            'items': {
                'type': 'object',
                'required': [ENDPOINT_TYPE, ENDPOINT_INTERFACE],
                'properties': {
                    ENDPOINT_TYPE: {
                        'type': 'string',
                        'enum': [ENDPOINT_TYPE_SOURCE, ENDPOINT_TYPE_SINK],
                    },
                    ENDPOINT_INTERFACE: {
                        'type': 'string',
                    },
                    ENDPOINT_ARGS: {
                        'type': 'array',
                        'default': []
                    },
                    ENDPOINT_KWARGS: {
                        'type': 'object',
                        'default': {}
                    },
                }
            }
        },
        OPERATOR: {
            'type': 'array',
            'uniqueItems': False,
            'default': [],
            'items': {
                'type': 'object',
                'required': [OPERATOR_INTERFACE],
                'properties': {
                    OPERATOR_INTERFACE: {
                        'type': ['integer', 'string'],
                    },
                    OPERATOR_ARGS: {
                        'type': 'array',
                        'default': []
                    },
                    OPERATOR_KWARGS: {
                        'type': 'object',
                        'default': {}
                    },
                    OPERATOR_CREATE: {
                        'type': 'object',
                        'properties': {
                            OPERATOR_ARGS: {
                                'type': 'array',
                                'default': []
                            },
                            OPERATOR_KWARGS: {
                                'type': 'object',
                                'default': {}
                            },
                        }
                    },
                }
            }
        },
        KOPERATOR: {
            'type': 'array',
            'uniqueItems': False,
            'default': [],
            'items': {
                'type': 'object',
                'required': [KOPERATOR_INTERFACE],
                'properties': {
                    KOPERATOR_INTERFACE: {
                        'type': 'string',
                    },
                    KOPERATOR_ARGS: {
                        'type': 'array',
                        'default': []
                    },
                    KOPERATOR_KWARGS: {
                        'type': 'object',
                        'default': {}
                    },
                }
            }
        },
        GRAPH: {
            'type': 'array',
            'uniqueItems': True,
            'default': [],
            'items': {
                'type': 'array',
                'minItems': 2,
                'maxItems': 2,
                'uniqueItems': True,
                'items': {
                    'type': 'string',
                }
            }
        },
        SYNC: {
            'type': 'array',
            'uniqueItems': True,
            'default': [],
            'items': {
                'type': 'array',
                'minItems': 2,
                'uniqueItems': True,
                'items': {
                    'type': 'string',
                }
            }
        }
    }
}

NODE_INPUT = 'i'
NODE_OUTPUT = 'o'

OPERATOR_INPUT = NODE_INPUT
OPERATOR_OUTPUT = NODE_OUTPUT

KOPERATOR_INPUT = NODE_INPUT
KOPERATOR_OUTPUT = NODE_OUTPUT

GRAPH_FROM = 'from'
GRAPH_TO = 'to'


class Node(object):
    def __init__(self, node_type, node_index, node_modifier=None):
        self._node_type = node_type
        self._node_index = node_index
        self._node_modifier = node_modifier if node_modifier else []

    def get_type(self):
        return self._node_type

    def get_index(self):
        return self._node_index

    def get_modifier(self, index=None):
        if index is None:
            return self._node_modifier
        else:
            return self._node_modifier[index]

    def __repr__(self):
        return ''.join([self._node_type, str(self._node_index)] +
                       [str(ent) for ent in self._node_modifier])


class Graph(object):
    '''
    Generic graph library

    This class handles all elements in a class (streams, endpoints, operators and koperators).

    Args:
        config (dict): Input data configuration
        koperator_enable (bool): Enable koperators
    '''

    def __init__(self, config, koperator_enable=True):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log

        self._config = copy.deepcopy(config)
        self._koperator_enable = koperator_enable
        self._internal_graph = []
        self._internal_sync = []

        self._log.info('validating against kalsim schema')
        DefaultValidatingDraft4Validator(INPUT_DATA_SCHEMA).validate(self._config)
        self._validate()

    def _check_node(self, node):
        '''
        Verify that a node name complies with the formula

        - streamX and that index X exists in the configuration
        - endpointX and that index X exists in the configuration
        - operatorXinY and that index X exists in the configuration
        - operatorXoY and that index X exists in the configuration
        - koperatorXinY and that index X exists in the configuration
        - koperatorXoY and that index X exists in the configuration

        Args:
            node (str): Node name

        Raises:
            ValueError: If the node is invalid
        '''
        node = str(node)  # unicode to str in python 2
        out = [''.join(g) for _, g in groupby(node, str.isalpha)]

        if out[0].lower().startswith(STREAM):
            if len(out) != 2:
                raise ValueError('invalid node %s' % (node))
            elif self.get_stream_num() <= int(out[1]):
                raise ValueError('node %s not found in configuration' % (node))
            return out[0], int(out[1]), []
        elif out[0].lower().startswith(ENDPOINT):
            if len(out) != 2:
                raise ValueError('invalid node %s' % (node))
            elif self.get_endpoint_num() <= int(out[1]):
                raise ValueError('node %s not found in configuration' % (node))
            return out[0], int(out[1]), []
        elif out[0].lower().startswith(OPERATOR):
            if len(out) != 4 or out[2].lower() not in [OPERATOR_INPUT, OPERATOR_OUTPUT]:
                raise ValueError('invalid node %s' % (node))
            elif self.get_operator_num() <= int(out[1]):
                raise ValueError('node %s not found in configuration' % (node))
            return out[0], int(out[1]), [out[2], int(out[3])]
        elif self._koperator_enable and out[0].lower().startswith(KOPERATOR):
            if len(out) != 4 or out[2].lower() not in [KOPERATOR_INPUT, KOPERATOR_OUTPUT]:
                raise ValueError('invalid node %s' % (node))
            elif self.get_koperator_num() <= int(out[1]):
                raise ValueError('node %s not found in configuration' % (node))
            return out[0], int(out[1]), [out[2], int(out[3])]
        else:
            raise ValueError('node %s invalid' % (node))

    def _validate_from_koperator(self, edge):
        '''
        Connections from a koperator validation

            - koperator should be output
            - koperator to stream source or koperator input

        Args:
            edge (list[dict,dict]): From and to nodes
        '''
        # from koperator has to be output
        if edge[GRAPH_FROM].get_modifier(0) != KOPERATOR_OUTPUT:
            raise ValueError('node %s out connections are not allowed' % (str(edge[GRAPH_FROM])))

        # FIXME check output does not exceed maximum for this koperator
        # unfortunately that requires connecting to capability_factory.get_class which is
        # kalsim specific

        # from koperator maximum output connections is 1
        connections = self.get_node_out_connections(KOPERATOR, edge[GRAPH_FROM].get_index(),
                                                    edge[GRAPH_FROM].get_modifier(1))
        if len(connections) > 1:
            raise ValueError('node %s connects to multiple points' % (str(edge[GRAPH_FROM])))

        # from koperator to stream must be stream source
        if edge[GRAPH_TO].get_type() == STREAM:
            _, stream_type, _, _ = self.get_endpoint(None, edge[GRAPH_TO].get_index())

            if stream_type != STREAM_TYPE_SOURCE:
                raise ValueError(
                    'node %s connected to %s only can connect to stream source' %
                    (str(edge[GRAPH_FROM]),
                     str(edge[GRAPH_TO])))

        # from koperator to koperator must be koperator input
        elif edge[GRAPH_TO].get_type() == KOPERATOR:
            if edge[GRAPH_TO].get_modifier(0) != KOPERATOR_INPUT:
                raise ValueError(
                    'node %s connected to %s only can connect to an input' %
                    (str(edge[GRAPH_FROM]),
                     str(edge[GRAPH_TO])))

        # no other cases allowed
        else:
            raise ValueError(
                'node %s connected to %s only can connect to %s or %s' %
                (str(edge[GRAPH_FROM]),
                 str(edge[GRAPH_TO]),
                 KOPERATOR,
                 STREAM))

    def _validate_from_stream(self, edge):
        '''
        Connections from a stream validation

            - stream source to endpoint source
            - stream sink to koperator input

        Args:
            edge (list[dict,dict]): From and to nodes
        '''
        _, stream_type, _, _ = self.get_stream(None, edge[GRAPH_FROM].get_index())

        # from streams maximum output connections is 1
        connections = self.get_node_out_connections(STREAM, edge[GRAPH_FROM].get_index())
        if len(connections) > 1:
            raise ValueError('node %s (%s) connects to multiple points' %
                             (str(edge[GRAPH_FROM]), stream_type))

        if stream_type == STREAM_TYPE_SOURCE:
            # from stream source can only be connected to endpoint source
            if edge[GRAPH_TO].get_type() != ENDPOINT:
                raise ValueError(
                    'node %s (%s) connected to %s only can connect to endpoint source' %
                    (str(edge[GRAPH_FROM]),
                     stream_type,
                     str(edge[GRAPH_TO])))

            _, endpoint_type, _, _ = self.get_endpoint(None, edge[GRAPH_TO].get_index())
            if endpoint_type != ENDPOINT_TYPE_SOURCE:
                raise ValueError(
                    'node %s (%s) connected to %s (%s) only can connect to endpoint source' %
                    (str(edge[GRAPH_FROM]),
                     stream_type,
                     str(edge[GRAPH_TO]),
                     endpoint_type))
        else:
            # from stream sink can only be connected to koperator input
            if edge[GRAPH_TO].get_type() != KOPERATOR:
                raise ValueError(
                    'node %s (%s) connected to %s only can connect to koperator' %
                    (str(edge[GRAPH_FROM]),
                     stream_type,
                     str(edge[GRAPH_TO])))

            if edge[GRAPH_TO].get_modifier(0) != KOPERATOR_INPUT:
                raise ValueError(
                    'node %s (%s) connected to %s only can connect to koperator input' %
                    (str(edge[GRAPH_FROM]),
                     stream_type,
                     str(edge[GRAPH_TO])))

    def _validate_from_endpoint(self, edge):
        '''
        Connections from an endpoint validation

            - endpoint source to operator input
            - endpoint sink to stream sink

        Args:
            edge (list[dict,dict]): From and to nodes
        '''
        _, endpoint_type, _, _ = self.get_endpoint(None, edge[GRAPH_FROM].get_index())

        # from endpoints maximum output connections is 1
        connections = self.get_node_out_connections(ENDPOINT, edge[GRAPH_FROM].get_index())
        if len(connections) > 1:
            raise ValueError('node %s (%s) connects to multiple points' %
                             (str(edge[GRAPH_FROM]), endpoint_type))

        if endpoint_type == ENDPOINT_TYPE_SOURCE:
            # from endpoint source can only be connected to operator
            if edge[GRAPH_TO].get_type() != OPERATOR:
                raise ValueError(
                    'node %s (%s) connected to %s only can connect operator' %
                    (str(edge[GRAPH_FROM]),
                     endpoint_type,
                     str(edge[GRAPH_TO])))

            if edge[GRAPH_TO].get_modifier(0) != OPERATOR_INPUT:
                raise ValueError(
                    'node %s (%s) connected to %s only can connect to operator input' %
                    (str(edge[GRAPH_FROM]),
                     endpoint_type,
                     str(edge[GRAPH_TO])))

        else:
            # from endpoint sink can only be connected to stream sink
            if edge[GRAPH_TO].get_type() != STREAM:
                raise ValueError(
                    'node %s (%s) connected to %s only can connect stream' %
                    (str(edge[GRAPH_FROM]),
                     endpoint_type,
                     str(edge[GRAPH_TO])))

            _, stream_type, _, _ = self.get_stream(None, edge[GRAPH_TO].get_index())
            if stream_type != STREAM_TYPE_SINK:
                raise ValueError(
                    'node %s (%s) connected to %s only can connect to stream sink' %
                    (str(edge[GRAPH_FROM]),
                     endpoint_type,
                     str(edge[GRAPH_TO])))

    def _validate_from_operator(self, edge):
        '''
        Connections from an operator validation

            - operator should be output
            - operator to endpoint sink
            - operator to operator input

        Args:
            edge (list[dict,dict]): From and to nodes
        '''
        # from operator has to be output
        if edge[GRAPH_FROM].get_modifier(0) != OPERATOR_OUTPUT:
            raise ValueError('node %s out connections are not allowed' % (str(edge[GRAPH_FROM])))

        # from operator maximum output connections is 1
        connections = self.get_node_out_connections(OPERATOR, edge[GRAPH_FROM].get_index(),
                                                    edge[GRAPH_FROM].get_modifier(1))
        if len(connections) > 1:
            raise ValueError('node %s connects to multiple points' % (str(edge[GRAPH_FROM])))

        # from operator to endpoint must be endpoint sink
        if edge[GRAPH_TO].get_type() == ENDPOINT:
            _, stream_type, _, _ = self.get_endpoint(None, edge[GRAPH_TO].get_index())

            if stream_type != STREAM_TYPE_SINK:
                raise ValueError(
                    'node %s connected to %s only can connect to endpoint sink' %
                    (str(edge[GRAPH_FROM]),
                     str(edge[GRAPH_TO])))

        # from operator to operator must be operator input
        elif edge[GRAPH_TO].get_type() == OPERATOR:
            if edge[GRAPH_TO].get_modifier(0) != OPERATOR_INPUT:
                raise ValueError(
                    'node %s connected to %s only can connect to an input' %
                    (str(edge[GRAPH_FROM]),
                     str(edge[GRAPH_TO])))

        # no other cases allowed
        else:
            raise ValueError(
                'node %s connected to %s only can connect to %s or %s' %
                (str(edge[GRAPH_FROM]),
                 str(edge[GRAPH_TO]),
                 ENDPOINT,
                 OPERATOR))

    def _validate_to_koperator(self, edge):
        '''
        Connections to a koperator validation

            - koperator should be input and input should be valid

        Args:
            edge (list[dict,dict]): From and to nodes
        '''
        # to koperator has to be input
        if edge[GRAPH_TO].get_modifier(0) != KOPERATOR_INPUT:
            raise ValueError('node %s in connections are not allowed' %
                             (str(edge[GRAPH_TO])))

        # to koperator maximum input connections is 1
        connections = self.get_node_in_connections(KOPERATOR, edge[GRAPH_TO].get_index(),
                                                   edge[GRAPH_TO].get_modifier(1))
        if len(connections) > 1:
            raise ValueError('node %s in connections from multiple points' % (str(edge[GRAPH_TO])))

        # FIXME check input does not exceed maximum for this koperator
        # unfortunately that requires connecting to capability_factory.get_class which is
        # kalsim specific

    def _validate_to_stream(self, edge):
        # to streams maximum input connections is 1
        connections = self.get_node_in_connections(STREAM, edge[GRAPH_TO].get_index())
        if len(connections) > 1:
            raise ValueError('node to %s connects from multiple points' %
                             (str(edge[GRAPH_FROM])))

    def _validate_to_endpoint(self, edge):
        # to endpoints maximum input connections is 1
        connections = self.get_node_in_connections(ENDPOINT,
                                                   edge[GRAPH_TO].get_index())
        if len(connections) > 1:
            raise ValueError('node to %s connects from multiple points' %
                             (str(edge[GRAPH_TO])))

    def _validate_to_operator(self, edge):
        '''
        Connections to an operator validation

            - operator should be input

        Args:
            edge (list[dict,dict]): From and to nodes
        '''
        # to operator has to be input
        if edge[GRAPH_TO].get_modifier(0) != OPERATOR_INPUT:
            raise ValueError('node to %s is not allowed, not an input' % (str(edge[GRAPH_TO])))

        # to operator maximum input connections is 1
        connections = self.get_node_in_connections(OPERATOR, edge[GRAPH_TO].get_index(),
                                                   edge[GRAPH_TO].get_modifier(1))
        if len(connections) > 1:
            raise ValueError('node %s in connections from multiple points' %
                             (str(edge[GRAPH_TO])))

    def _validate(self):
        internal_graph = []
        for entry in self._config[GRAPH]:
            data = {}
            node_type, number, modifier = self._check_node(entry[0])
            data[GRAPH_FROM] = Node(node_type, number, modifier)

            node_type, number, modifier = self._check_node(entry[1])
            data[GRAPH_TO] = Node(node_type, number, modifier)

            internal_graph.append(data)

        self._internal_graph = internal_graph

        internal_sync = []
        for entry in self._config[SYNC]:
            data = []
            for node in entry:
                node_type, number, modifier = self._check_node(node)
                if node_type != ENDPOINT:
                    raise RuntimeError('sync node %s invalid, should be %s' % (node, ENDPOINT))
                data.append(Node(node_type, number, modifier))

            internal_sync.append(data)

        self._internal_sync = internal_sync

        for edge in self._internal_graph:
            if edge[GRAPH_FROM].get_type() == KOPERATOR:
                self._validate_from_koperator(edge)
            elif edge[GRAPH_FROM].get_type() == STREAM:
                self._validate_from_stream(edge)
            elif edge[GRAPH_FROM].get_type() == ENDPOINT:
                self._validate_from_endpoint(edge)
            elif edge[GRAPH_FROM].get_type() == OPERATOR:
                self._validate_from_operator(edge)

            if edge[GRAPH_TO].get_type() == KOPERATOR:
                self._validate_to_koperator(edge)
            elif edge[GRAPH_TO].get_type() == STREAM:
                self._validate_to_stream(edge)
            elif edge[GRAPH_TO].get_type() == ENDPOINT:
                self._validate_to_endpoint(edge)
            elif edge[GRAPH_TO].get_type() == OPERATOR:
                self._validate_to_operator(edge)

    def get_node_out_connections(self, node_type, node_num, output_num=None):
        res = []
        for entry in self._internal_graph:
            if (entry[GRAPH_FROM].get_type() == node_type and
                    entry[GRAPH_FROM].get_index() == node_num and
                    (output_num is None or (entry[GRAPH_FROM].get_modifier(0) == NODE_OUTPUT and
                                            entry[GRAPH_FROM].get_modifier(1) == output_num))):
                res.append(entry)
        return res

    def get_node_in_connections(self, node_type, node_num, input_num=None):
        res = []
        for entry in self._internal_graph:
            if (entry[GRAPH_TO].get_type() == node_type and
                    entry[GRAPH_TO].get_index() == node_num and
                    (input_num is None or (entry[GRAPH_TO].get_modifier(0) == NODE_INPUT and
                                           entry[GRAPH_TO].get_modifier(1) == input_num))):
                res.append(entry)
        return res

    def check_node_in_graph(self, node_type, node_index):
        '''
        Verifies that a node is in the graph.
        Nodes not defined in the graph will not be created
        '''
        ret = False
        for entry in self._internal_graph:
            for ent in [GRAPH_FROM, GRAPH_TO]:
                if entry[ent].get_type() == node_type and entry[ent].get_index() == node_index:
                    ret = True
                    break
        return ret

    def get_graph_connections(self, index=None):
        '''
        Get graph connections as nodes

        Args:
            index (int): Zero-based graph index

        Returns:
            list:
                Node: From node
                Node: To node
        '''
        if index is None:
            return self._internal_graph
        else:
            num = 0
            for entry in self._internal_graph:
                if num == index:
                    return entry
                else:
                    num += 1
        raise ValueError('graph index:%s not found' % (index))

    def get_sync_connections(self, index=None):
        '''
        Get sync connections as nodes

        Args:
            index (int): Zero-based sync index

        Returns:
            list[Node]: Nodes
        '''
        if index is None:
            return self._internal_sync
        else:
            num = 0
            for entry in self._internal_sync:
                if num == index:
                    return entry
                else:
                    num += 1
        raise ValueError('sync index:%s not found' % (index))

    def get_stream(self, stream_type, index):
        '''
        Get stream configuration entry

        Args:
            stream_type (str): Stream type ('source' or 'sink' or None)
            index (int): Zero-based index

        Returns:
            tuple:
                str: Interface
                str: Type (source or sink)
                list: Positional arguments
                OrderedDict: Keyword arguments
        '''
        num = 0
        for entry in self._config[STREAM]:
            if not stream_type or entry[STREAM_TYPE] == stream_type:
                if num == index:
                    return entry[STREAM_INTERFACE], entry[STREAM_TYPE], \
                           entry[STREAM_ARGS], entry[STREAM_KWARGS]
                else:
                    num += 1
        raise ValueError('stream_type:%s index:%s not found' % (stream_type, index))

    def get_stream_num(self, stream_type=None):
        '''
        Get stream configuration entry

        Args:
            stream_type (str): Stream type ('source' or 'sink' or None)

        Returns:
            int: Number of streams found
        '''
        num = 0
        for entry in self._config[STREAM]:
            if not stream_type or entry[STREAM_TYPE] == stream_type:
                num += 1
        return num

    def get_stream_method(self, stream_type, index, method):
        '''
        Get stream configuration entry for an specific method

        Args:
            stream_type (str): Stream type ('source' or 'sink' or None)
            index (int): Zero-based endpoint index
            method (str): Method configuration is requested

        Returns:
            tuple:
                list: Positional arguments
                OrderedDict: Keyword arguments
        '''
        num = 0
        for entry in self._config[STREAM]:
            if not stream_type or entry[STREAM_TYPE] == stream_type:
                if num == index:
                    if method in entry:
                        return entry[method][STREAM_ARGS], entry[method][STREAM_KWARGS]
                    else:
                        return [], {}
                else:
                    num += 1
        raise ValueError('stream_type:%s index:%s not found' % (stream_type, index))

    def get_endpoint(self, endpoint_type, index):
        '''
        Get endpoint configuration entry

        Args:
            endpoint_type (str): Endpoint type ('source' or 'sink' or None)
            index (int): Zero-based endpoint index

        Returns:
            tuple:
                str: Interface
                str: Type (source or sink)
                list: Positional arguments
                OrderedDict: Keyword arguments
        '''
        num = 0
        for entry in self._config[ENDPOINT]:
            if not endpoint_type or entry[ENDPOINT_TYPE] == endpoint_type:
                if num == index:
                    return entry[ENDPOINT_INTERFACE], entry[ENDPOINT_TYPE], \
                           entry[ENDPOINT_ARGS], entry[ENDPOINT_KWARGS]
                else:
                    num += 1
        raise ValueError('endpoint_type:%s index:%s not found' % (endpoint_type, index))

    def get_endpoint_num(self, endpoint_type=None):
        '''
        Get endpoint configuration entry

        Args:
            endpoint_type (str): Endpoint type ('source' or 'sink' or None)

        Returns:
            int: Number of endpoints found
        '''
        num = 0
        for entry in self._config[ENDPOINT]:
            if not endpoint_type or entry[ENDPOINT_TYPE] == endpoint_type:
                num += 1
        return num

    def get_endpoint_method(self, endpoint_type, index, method):
        '''
        Get endpoint configuration entry for an specific method

        Args:
            endpoint_type (str): Endpoint type ('source' or 'sink' or None)
            index (int): Zero-based endpoint index
            method (str): Method configuration is requested

        Returns:
            tuple:
                list: Positional arguments
                OrderedDict: Keyword arguments
        '''
        num = 0
        for entry in self._config[ENDPOINT]:
            if not endpoint_type or entry[ENDPOINT_TYPE] == endpoint_type:
                if num == index:
                    if method in entry:
                        return entry[method][ENDPOINT_ARGS], entry[method][ENDPOINT_KWARGS]
                    else:
                        return [], {}
                else:
                    num += 1
        raise ValueError('endpoint_type:%s index:%s not found' % (endpoint_type, index))

    def get_operator(self, index):
        '''
        Get operator configuration entry

        Args:
            index (int): Zero-based operator index

        Returns:
            tuple:
                str: Interface
                list: Positional arguments
                OrderedDict: Keyword arguments
        '''
        num = 0
        for entry in self._config[OPERATOR]:
            if num == index:
                return entry[OPERATOR_INTERFACE], entry[OPERATOR_ARGS], entry[OPERATOR_KWARGS]
            else:
                num += 1
        raise ValueError('operator index:%s not found' % (index))

    def get_operator_num(self):
        '''
        Get operator configuration entry

        Returns:
            int: Number of operators found
        '''
        return len(self._config[OPERATOR])

    def get_operator_method(self, index, method):
        '''
        Get operator configuration entry for an specific method

        Args:
            index (int): Zero-based operator index
            method (str): Method configuration is requested

        Returns:
            tuple:
                list: Positional arguments
                OrderedDict: Keyword arguments
        '''
        num = 0
        for entry in self._config[OPERATOR]:
            if num == index:
                if method in entry:
                    return entry[method][OPERATOR_ARGS], entry[method][OPERATOR_KWARGS]
                else:
                    return [], {}
            else:
                num += 1
        raise ValueError('operator index:%s not found' % (index))

    def get_koperator(self, index):
        '''
        Get kats operator configuration entry

        Args:
            index (int): Zero-based operator index

        Returns:
            tuple:
                str: Interface
                list: Positional arguments
                OrderedDict: Keyword arguments
        '''
        num = 0
        for entry in self._config[KOPERATOR]:
            if num == index:
                return entry[KOPERATOR_INTERFACE], entry[KOPERATOR_ARGS], entry[KOPERATOR_KWARGS]
            else:
                num += 1
        raise ValueError('operator index:%s not found' % (index))

    def get_koperator_num(self):
        '''
        Get kats operator configuration entry

        Returns:
            int: Number of operators found
        '''
        return len(self._config[KOPERATOR])

    def get_koperator_method(self, index, method):
        '''
        Get kats operator configuration entry for an specific method

        Args:
            index (int): Zero-based operator index
            method (str): Method configuration is requested

        Returns:
            tuple:
                list: Positional arguments
                OrderedDict: Keyword arguments
        '''
        num = 0
        for entry in self._config[KOPERATOR]:
            if num == index:
                if method in entry:
                    return entry[method][KOPERATOR_ARGS], entry[method][KOPERATOR_KWARGS]
                else:
                    return [], {}
            else:
                num += 1
        raise ValueError('operator index:%s not found' % (index))

    def get_graph(self, index=None):
        '''
        Get graph entry

        Args:
            index (int): Zero-based graph index

        Returns:
            list[[str,str]]: Graph configuration
        '''
        if index is None:
            return self._config[GRAPH]
        else:
            num = 0
            for entry in self._config[GRAPH]:
                if num == index:
                    return entry
                else:
                    num += 1
        raise ValueError('graph index:%s not found' % (index))

    def get_graph_num(self):
        '''
        Get number of graph entries

        Returns:
            int: Number of graphs found
        '''
        return len(self._config[GRAPH])

    def get_sync(self, index=None):
        '''
        Get sync entry

        Args:
            index (int): Zero-based sync index

        Returns:
            list[[str]]: Sync configuration
        '''
        if index is None:
            return self._config[SYNC]
        else:
            num = 0
            for entry in self._config[SYNC]:
                if num == index:
                    return entry
                else:
                    num += 1
        raise ValueError('sync index:%s not found' % (index))

    def get_sync_num(self):
        '''
        Get number of sync entries

        Returns:
            int: Number of sync found
        '''
        return len(self._config[SYNC])
