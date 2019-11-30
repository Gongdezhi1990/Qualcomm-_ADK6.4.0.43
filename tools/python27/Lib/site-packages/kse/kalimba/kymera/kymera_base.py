'''
Kymera Base class
'''

from abc import ABCMeta, abstractmethod, abstractproperty

from six import add_metaclass


@add_metaclass(ABCMeta)
class KymeraBase(object):
    '''
    Kymera abstract class

    This class should be subclassed by all possible kymera implementation to give a compatible
    API for the rest of the system

    Args:
        uut (kats.kalimba.uut.uut_base.UutBase): Uut instance
    '''

    def __init__(self, uut, *_, **__):
        pass

    @abstractproperty
    def platform(self):
        '''
        str: Platform name
        '''
        pass

    @abstractproperty
    def interface(self):
        '''
        str: Interface name
        '''
        pass

    @abstractmethod
    def opmgr_create_operator(self, cap_id):
        '''
        Creates an operator in the stopped mode with endpoints

        Args:
            cap_id (int): Id of the capability to instantiate

        Returns:
            int: Operator id
        '''
        pass

    @abstractmethod
    def opmgr_create_operator_ex(self, cap_id, priority=0, processor_id=0):
        '''
        Creates an operator (parameterised version) in the stopped mode with endpoints

        Args:
            cap_id (int): Id of the capability to instantiate
            priority (int): Priority, 0 is the lowest
            processor_id (int): Processor, 0 is main

        Returns:
            int: Operator id
        '''
        pass

    @abstractmethod
    def opmgr_destroy_operators(self, operators):
        '''
        When the operator is stopped, this commands destroys it and frees all of its resources

        Args:
            operators (list[int]): Id of the operators to destroy
        '''
        pass

    @abstractmethod
    def opmgr_start_operators(self, operators):
        '''
        Starts an operator and any endpoints connected to the operator, creating a data flow and
        data processing

        Args:
            operators (list[int]): Id of the operators to start
        '''
        pass

    @abstractmethod
    def opmgr_stop_operators(self, operators):
        '''
        Stops an operator

        Args:
            operators (list[int]): Id of the operators to stop
        '''
        pass

    @abstractmethod
    def opmgr_reset_operators(self, operators):
        '''
        Reinitialize an operator

        Args:
            operators (list[int]): Id of the operators to reset
        '''
        pass

    @abstractmethod
    def opmgr_operator_message(self, op_id, msg):
        '''
        Send a message to the specified operator containing addiitional information,
        such as configuration data, where the message does not destroy the operator or
        affect its state

        Args:
            op_id (int): Id of the operator
            msg (list[int]): Message to be sent

        Returns:
            list[int]: Message response
        '''
        pass

    @abstractmethod
    def opmgr_get_capid_list(self):
        '''
        Request a list of the capability IDs supported by the device

        Returns:
            list[int]: Capability ID
        '''
        pass

    @abstractmethod
    def opmgr_get_opid_list(self, cap_id=0):
        '''
        Request a list of operator IDs supported by Kymera that are provided to the client
        including the capability IDs that instantiated the operators

        Args:
            cap_id (int): Capability ID or 0 for all capabilities

        Returns:
            list[int]: Operator ID
        '''
        pass

    @abstractmethod
    def opmgr_get_cap_info(self, cap_id):
        '''
        Request the information structure for the specified capability from kymera

        Args:
            cap_id (int): Capability ID

        Returns:
            list[int]: Information
        '''
        pass

    @abstractmethod
    def stream_if_get_source(self, endpoint_type, params):
        '''
        Create a source endpoint of the specified type, including all parameters specific to that
        type.
        Extra information can be passed to the created endpoint using configuration commands.

        Args:
            endpoint_type(int) : Type of endpoint
            params (list[int]): Endpoint parameters

        Returns:
            int: Source endpoint id
        '''
        pass

    @abstractmethod
    def stream_if_get_sink(self, endpoint_type, params):
        '''
        Create a sink endpoint of the specified type, including all parameters specific to that
        type.
        Extra information can be passed to the created endpoint using configuration commands.

        Args:
            endpoint_type (int) : Type of endpoint
            params (list[int]): Endpoint parameters

        Returns:
            int: Sink endpoint id
        '''
        pass

    @abstractmethod
    def stream_if_close_source(self, source_id):
        '''
        Close a source endpoint that was created using stream_if_get_source command.

        Args:
            source_id (int) : Source endpoint id

        Returns:
            int: Source endpoint id
        '''
        pass

    @abstractmethod
    def stream_if_close_sink(self, sink_id):
        '''
        Close a sink endpoint that was created using stream_if_get_sink command.

        Args:
            sink_id (int): Endpoint id
        '''
        pass

    @abstractmethod
    def stream_if_configure_sid(self, ep_id, key, value):
        '''
        Configure an already created endpoint

        Args:
            ep_id (int): Endpoint id
            key (str or int): id specific to the endpoint
            value (any): Value of the parameter to be changed
        '''
        pass

    @abstractmethod
    def stream_if_connect(self, source_id, sink_id):
        '''
        Connect a source and a sink to create a transform with a unique numeric id,
        where either source or sink can be the endpoint or terminal.
        The command will fail if either

        - Source or sink is already connected
        - Source or sink id argument does not match the respective component real direction

        If both source and sink have the same data type, for example, pcm samples or encoded data,
        the connection will always work.
        If data types are incompatible and the conversion is not completed automatically,
        then manual data conversion is required.

        Args:
            source_id (int): Source endpoint id
            sink_id (int): Sink endpoint id

        Returns:
            int: Transform id
        '''
        pass

    @abstractmethod
    def stream_if_transform_disconnect(self, transforms):
        '''
        Disconnect one or more transforms.

        Args:
            transforms (list[int]): Transforms id
        '''
        pass

    @abstractmethod
    def stream_if_sync_sids(self, ep1, ep2):
        '''
        Synchronize two endpoints in terms of their data flow, for example,
        two endpoints of a stereo audio feed, and add the specified endpoints be a list.
        It is possible to synchronize more than two endpoints, by using them in overlapping pairs in
        successive stream_if_sync_sids() calls, for example, synchronize:

        - a. (ep1, ep2)
        - b. (ep2, ep3)
        - c. (ep3, ep4)

        If any of the specified endpoint IDs is zero, then the associated endpoint is
        removed from the synchronized list.
        Closing an endpoint also removes that endpoints synchronization.
        When specifying endpoints to synchronize:

        - Only synchronize endpoints of the same type, for example
        - If any attempt is made to synchronize two endpoints different types,
          then the operation will fail with STATUS_CMD_FAILED status code.
        - Receipt of a terminal ID as source or sink ID is tolerated without error, however a
          synchronization attempt of this type will have no effect

        Args:
            ep1 (int): First endpoint id. If zero it has a unsync effect, it removes the other
                endpoint from the synchronized group
            ep2 (int): Second endpoint id. If zero it has a unsync effect, it removes the other
                endpoint from the synchronized group
        '''
        pass

    @abstractmethod
    def stream_get_connection_list(self, source_id, sink_id):
        '''
        Request a list of transform ids and the ids of the transform endpoints from Kymera.

        Args:
            source_id (int): Source endpoint id. If non-zero only transforms that have matching
                source will be returned
            sink_id (int): Sink endpoint id. If non-zero only transforms that have matching sink
                will be returned

        Returns:
            list[tuple]: Info list triplet with transform id, source id and sink id
        '''
        pass
