/****************************************************************************
Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd.
*/

/**
\file chain.h

\brief Chain library for management of Kymera operator groups 

# Introduction

The Chain library creates an abstraction layer on top of APIs provided
by the Operators library and the Apps P0 firmware APIs. The purpose of the 
library is to simplify management of groups of Kymera operators which are 
used together to implement a more complex functionality (e.g. USB audio 
input with rate matching).

# Principle of operation

The library operates on operator "chains". Each chain is defined by 
chain_config_t structure. The structures are typically defined as constant
and MAKE_CHAIN_CONFIG_WITH_PATHS(id, ucid, operators, paths) macro is
provided to aid initialisation of the structures. Each chain instance is 
processed independently of other chains and contextual information - the 
library only operates on data stored in the structure describing the particular 
chain.   

## Chain description

An operator chain is defined by describing the data paths through the chain.
Macro MAKE_CHAIN_CONFIG_WITH_PATHS(id, ucid, operators, paths) can be used
to initialise the chain_config_t structure with constant data.
@dot digraph Current { 
    chain_config_t [URL="structchain__config__t.html" shape=box style=filled, fillcolor=lightgrey];
    operators [shape=box];
    paths [shape=box];
    chain_config_t -> operators [style=dashed,color="magenta"]; 
    chain_config_t -> paths [style=dashed,color="magenta"]; 
}
@enddot

\note
  It is also possible to to define a chain by describing its inputs, outputs 
  and internal connections between the operators. This is provided for backwards 
  compatibility with ADK 6.1 and should be avoided in new code.

### Operator description

@dot digraph Current { 
    chain_config_t [URL="structchain__config__t.html" shape=box];
    operators [shape=box, style=filled, fillcolor=lightgrey];
    paths [shape=box];
    chain_config_t -> operators [style=dashed,color="magenta"]; 
    chain_config_t -> paths [style=dashed,color="magenta"]; 
}
@enddot

Each operator chain structure must point to an array of operator_config_t
structures. This array describes all the operators contained in the chain.

Each operator is assigned a role. This is an arbitrary integer value which 
uniquely identifies the operator within the context of the chain. Operators 
cannot be uniquely identified by their capability ID as a single chain may
contain more than one instance of a single capability and the \ref filtering 
feature allows for an operator to be substituted for another during chain 
creation.

Operator description also includes an optional set-up information contained in
a operator_setup_t structure. The set-up data is sent to the operator after it 
is created in form of messages. Alternatively (in more complexe cases) the 
operator can be configured after the chain is created.  

### Path description

@dot digraph Current { 
    chain_config_t [URL="structchain__config__t.html" shape=box];
    operators [shape=box];
    paths [shape=box, style=filled, fillcolor=lightgrey];
    nodes [shape=box]; 
    chain_config_t -> operators [style=dashed,color="magenta"]; 
    chain_config_t -> paths [style=dashed,color="magenta"];
    paths -> nodes [style=dashed,color="magenta"];  
}
@enddot

Path describes the logical data flow through a chain of operators (e.g path
for right audio channel samples). A path can (but does not have to) originate 
and/or terminate outside of the chain, thus forming the input and/or output 
terminals of the chain (Sink and/or Source). A path can also originate and/or
terminate internally within the chain. The following figure depicts an example 
of a chain containing three different paths (differentiated by the colour of the 
arrows).
\anchor path_example
@dot
digraph PathsExample {
    rankdir=LR;
    subgraph cluster_inputs {
        color="none";
        left_sink [shape=none label="sink"];
        right_sink [shape=none label="sink"];
        { rank=same; left_sink, right_sink }
    }
    subgraph cluster_op1 {
        label="Operator A";
        opA_left_in [shape=box label="left input terminal"];    
        opA_left_out [shape=box label="left output terminal"];    
        opA_right_in [shape=box label="right input terminal"];    
        opA_right_out [shape=box label="right output terminal"];    
        { rank=same; opA_left_in, opA_right_in }
        { rank=same; opA_left_out, opA_right_out }        
        left_sink -> opA_left_in [color="blue"];
        right_sink -> opA_right_in [color="green"];  
        opA_left_out -> opB_left_in [color="blue"];
        opA_right_out -> opB_right_in [color="green"];  
    }
    subgraph cluster_outputs {
        color="none"; 
        data_source [shape=none label="source"];
    }
    subgraph cluster_op3 {
        label="Operator C";
        opC_left_in [shape=box label="left input terminal"];    
        opC_right_in [shape=box label="right input terminal"];
    }    
    subgraph cluster_op2 {
        label="Operator B";
        opB_left_in [shape=box label="left input terminal"];    
        opB_left_out [shape=box label="left output terminal"];
        opB_right_in [shape=box label="right input terminal"];    
        opB_right_out [shape=box label="right output terminal"];    
        { rank=same; opB_left_in, opB_right_in }
        { rank=same; opB_left_out, opB_right_out }        
        opB_left_out -> opC_left_in [color="blue"];
        opB_right_out -> opC_right_in [color="green"];
          
    }
    subgraph cluster_op3 {
        opC_data_out [shape=box label="data output terminal"];    
        { rank=same; opC_left_in, opC_right_in }
        opC_data_out -> data_source [color="red"];        
    }
    
}
@enddot

Paths within a chain of operators are described by an array of operator_path_t
structures. As specified by the type member, each path can:

 - Be internal to the chain (originating and terminating in internal operator terminals)
 - Have its beginning exposed as a chain input (the first node of the path specifies an 
operator terminal which can be connected to a source external to the chain)
 - Have its end exposed as a chain output (the last node specifies output of the chain)
 - Have both its input and output exposed as chain input and output
 
Each path is described by an array of operator_path_node_t structures.
This array identifies the operators the path goes through. For each operator it describes
the internal data flow by specifying the input terminal (through which the data enters 
the operator) and the corresponding output terminal (through which the data exits 
the operator after it is processed).

The input terminal of the first node in the array and/or the output terminal of the 
last node in the array are ignored when the path originates and/or terminates
within the chain without defining a chain input and/or output.

For example the blue path in the example above would be defined as:

\code
{
    blue_role,          // a value uniquely identifying this path within the context of the chain
    path_with_input,    // this path forms and input of the chain, but terminates internaly within the chain
    3,                  // the path has 3 nodes
    -> {                // address of an array containing the following 3 node structures:
        {Operator A role, left input terminal, left output terminal},
        {Operator B role, left input terminal, left output terminal},
        {Operator C role, left input terminal, ignored value}
    }
}
\endcode     

# Lifetime of a chain

@dot
digraph ChainLifetime {
    chain_does_not_exist [label="Chain does not exist"]
    chain_created [label="DSP processors required by the chain started\nDownloadable capabilities loaded\nOperators instantiated\nOperators set-up"]
    chain_configured [label="Operators configured as required"]
    chain_connected [label="Internal nodes on all chain paths connected"]
    chain_connected_extern [label="Inputs and outputs connected"]
    chain_started [label="Chain operators started"]
    chain_does_not_exist -> chain_created [label="ChainCreate() or\nChainCreateWithFilter()"]
    chain_created -> chain_connected [label="ChainConnect()"]
    chain_configured -> chain_connected [label="ChainConnect()"]
    chain_created -> chain_configured [label="ChainConfigure()"]
    chain_connected -> chain_connected_extern [label="ChainConnectInput() &\nChainConnectOutput()"]
    chain_connected_extern -> chain_started [label="ChainStart()"]
    chain_started -> chain_connected_extern [label="ChainStop()"]
    chain_connected_extern -> chain_does_not_exist [label="ChainDestroy()"]
            
}
@enddot

\anchor filtering
## Filtering

Chains can be created exactly as defined in the chain_config_t structure using 
ChainCreate().
Alternatively a chain can be created with some modifications applied to the 
chain_config_t structure using ChainCreateWithFilter(). This applies one or more 
filters to the chain definition before the chain is created thus allowing
one or more operators in the chain to be replaced or removed.

Examples of where filters are useful include:

 - Set-up of A2DP decoding chain changing the default SBC decoder for AAC decoder.
 - Set-up of line-in audio processing path leaving out sample rate adjustment
   when the input sample rate is defined by a local clock source and constant.
  
Filters do not add a fundamental new functionality which would not be achievable
without them. Their use avoids the need to define several very similar chains 
with minor modifications thus saving constant space and making the code easier
to maintain.  

The filters are specified by an operator_filters_t structure which is nothing more
than an array of operator_config_t structures with length. Each filter is matched
to an operator in the chain definition by having an equal role. 

### Replacing an operator

Once the filter is matched with an operator inside the chain, the attributes from 
the filter definition replace the attributes of the original operator.
The functionality of the original and replacement operators must be compatible
in terms of the paths which define how the operator is connected to the rest of
the chain - the operator terminal numbers contained in any node definitions must 
be applicable to both operators.  

### Removing an operator       

Setting the capability ID of an operator to capability_id_none in the filter 
definition results in the operator with a matching role being removed from 
the chain altogether. 

An operator can only be removed from a chain if there is no path terminating in
its input terminals and no path originating from its output terminals. 

Removing the first/last operator in a chain is supported (subject to the 
aforementioned restriction). ChainGetInput() and ChainGetOutput() will use the 
first and last valid operator in the chain respectively.  

E.g. in the \ref path_example "path example" operators A and/or B can be removed,
but operator C cannot. For example removing operator A would result in the the 
"left input terminal" and "right input terminal" of operator B forming the inputs
of the chain.  

## Downloadable capabilities

The chain library automates dowloading of capability bundles. When a chain 
uses one or more downloadable capabilities, the chain library will automatically 
download the required downloadable capability bundles.

The application has to specify which files in the read only filesystem contain
downloadable capability bundles by calling ChainSetDownloadableCapabilityBundleConfig().
 
# Examples

Definition of operator chain for decoding A2DP audio:
 
\code
static const operator_config_t op_config[] =
{
    MAKE_OPERATOR_CONFIG(capability_id_rtp, rtp_role),
    MAKE_OPERATOR_CONFIG(capability_id_splitter, splitter_role),
    MAKE_OPERATOR_CONFIG(capability_id_sbc_decoder, decoder_role)
};
 
static const chain_config_t a2dp_chain_config =
        MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_input_a2dp, audio_ucid_input_a2dp, op_config, paths);
\endcode

Instantiation of the A2DP decoder chain with SBC decoder:

\code
ChainCreate(a2dp_chain_config);
\endcode

Instantiation of the A2DP decoder chain with AAC decoder:

\code
static const operator_config_t aac_filter_list[] = 
{
    MAKE_OPERATOR_CONFIG(capability_id_aac_decoder, decoder_role)
};

static const operator_filters_t aac_filters =
{
    .num_operator_filters = sizeof(aac_filter_list)/sizeof(operator_config_t),
    .operator_filters = aac_filter_list
};
 
ChainCreateWithFilter(a2dp_chain_config, aac_filters); 
\endcode
 
*/

#ifndef LIBS_CHAIN_CHAIN_H_
#define LIBS_CHAIN_CHAIN_H_

#include <csrtypes.h>
#include <sink.h>
#include <source.h>
#include <operators.h>
#include <vmtypes.h>
#include <custom_operator.h>
#include <hydra_macros.h>
#include <audio_ucid.h>

/*! \internal Chain IDs are only used for test purposes
*/
typedef enum _chain_id
{
    chain_id_none,
    chain_id_audio_input_a2dp,
    chain_id_audio_input_i2s,
    chain_id_audio_input_analogue,
    chain_id_audio_input_common_forwarding,
    chain_id_audio_input_tws,
    chain_id_audio_input_usb,
    chain_id_audio_input_voice_prompts_tone,
    chain_id_audio_input_voice_prompts_decoder,
    chain_id_audio_input_voice_prompts_dummy,
    chain_id_audio_input_spdif,
    chain_id_audio_input_broadcast,
    chain_id_audio_hardware_aec,
    chain_id_audio_mixer_core,
    chain_id_audio_mixer_voice_resampler,
    chain_id_audio_mixer_music_resampler,
    chain_id_audio_mixer_prompt_resampler,
    chain_id_audio_mixer_voice_processing,
    chain_id_audio_mixer_music_processing,
    chain_id_audio_mixer_prompt_processing,
    chain_id_audio_mixer_speaker,
    chain_id_cvc_common,
    chain_id_audio_output_tws,
    chain_id_aov,
    chain_id_tuning_mode,
    chain_id_va,
    FORCE_ENUM_TO_MIN_16BIT(chain_id_t),
} chain_id_t;

/*! Describes an operator as an element of a chain.
*/
typedef struct
{
    /*! ID of a capability to be instantiated when the operator is created. */ 
    capability_id_t capability_id;
    /*! Role of the operator in the chain.
        This can be any number/enum value as long as it is unique within the chain.
        The library does not interpret the value in any way. The value is used
        to refer to the particular operator within the chain.
     */
    unsigned role;
    /*! Processor ID identifies which Audio subsystem processor the operator 
        should be available on.
     */
    operator_processor_id_t processor_id;
    /*! Scheduling priority of the operator within the Kymera framework. */
    operator_priority_t priority;
    /*! Configuration data sent to the operator (in form of messages) after it is 
       created.
    */ 
    operator_setup_t setup;
} operator_config_t;

/*! \deprecated Possible values for the inputs and outputs arrays in chain_config_t */ 
typedef struct
{
    unsigned operator_role;
    unsigned endpoint_role;
    unsigned terminal;
} operator_endpoint_t;

/*! \deprecated Possible values for the connections array in chain_config_t */ 
typedef struct
{
    unsigned source_role;
    unsigned first_source_terminal;

    unsigned sink_role;
    unsigned first_sink_terminal;

    unsigned number_of_terminals;
} operator_connection_t;

/*! Defines path type */
typedef enum
{
    /*! Path originates and terminates inside the chain */
    path_with_no_in_or_out = 0,
    /*! Path originates outside of the chain, but terminates inside the chain */
    path_with_input        = 1 << 0,
    /*! Path originates inside the chain, but terminates outside of the chain */
    path_with_output       = 1 << 1,
    /*! Path both originates and terminates outside of the chain */    
    path_with_in_and_out   = (path_with_input | path_with_output)
} path_type_t;

/*! Describes a node as part of a data path (see operator_path_t)
 */
typedef struct
{
    /*! Identifies one of the operators in the chain by its role.
        See operator_config_t for more details.
     */  
    unsigned operator_role;
    /*! Operator input terminal number. */ 
    unsigned input_terminal;
    /*! Operator output terminal number */
    unsigned output_terminal;
} operator_path_node_t;

/*! Describes data path through an operator chain.
 */
typedef struct
{
    /*! Role of the path in the chain.
        This can be any number/enum value as long as it is unique within the chain. 
        The library does not interpret the value in any way. The value is used
        to refer to the particular path within the chain.
     */
    unsigned path_role;
    /*! Type of path. Specifies whether the path forms input and/or output 
        of the chain (Sink and/or Source).
     */ 
    path_type_t type;
    /*! The number of operators path connects (size of nodes array) */ 
    unsigned number_of_nodes;
    /*! Pointer to an array of operator_path_node_t structures. Each node
        identifies an operator by its role, its input terminal and its output 
        terminal. Where the path type indicates that the path does not
        form input of the chain the input terminal specified by the first node
        in the array is ignored. Similarly the output terminal specified by the 
        last node in the array is ignored for a path which does form
        output of the chain. 
     */    
    const operator_path_node_t* nodes;
} operator_path_t;

/*! Decribes a set of filters to be used with ChainCreateWithFilter()
 */ 
typedef struct
{
    /*! Number of filters contained in the operator_filters array */ 
    unsigned num_operator_filters;
    /*! Pointer to an array of operator filters */ 
    const operator_config_t* operator_filters;
} operator_filters_t;

/*! Message to be sent to an operator after it is created
    Note that there is no support for messages with response */
typedef struct
{
    /*! The operator role to which the message will be sent */
    unsigned operator_role;
    /*! The message content */
    const uint16 *message;
    /*! The length of the message in uint16 units */
    const uint16 message_length; 
} chain_operator_message_t;

typedef struct
{
    /*! Chain ID is only used in library tests. */
    /*! The field can be used to enhance readability of code by providing an 
        enum value with a descriptive name. */
    chain_id_t chain_id;
    /*! ID of the Audio Use Case the chain participates in. */  
    /*! Different use cases have different resource requirements and the 
        presence/absence of a particular use case is used to set-up the Audio 
        subsystem accordingly. */
    audio_ucid_t audio_ucid;
    /*! Pointer to an array describing all operators in the chain. */
    const operator_config_t *operator_config;
    /*! Number of members of operator_config array. */ 
    unsigned number_of_operators;
    /*! \deprecated Pointer to an array describing all chain inputs. */
    const operator_endpoint_t *inputs;
    /*! \deprecated Number of members of inputs array. */  
    unsigned number_of_inputs;
    /*! \deprecated Pointer to an array describing all chain outputs. */
    const operator_endpoint_t *outputs;
    /*! \deprecated Number of members of outputs array. */
    unsigned number_of_outputs;
    /*! \deprecated Pointer to an array describing internal chain connections. */
    const operator_connection_t *connections;  
    /*! \deprecated Number of members of connections array. */
    unsigned number_of_connections;
    /*! Pointer to an array describing data paths inside the chain. */
    const operator_path_t *paths;
    /*! Number of members of paths array. */
    unsigned number_of_paths;
} chain_config_t;

typedef struct _chain_join_roles
{
    unsigned source_role;
    unsigned sink_role;
} chain_join_roles_t;

typedef struct kymera_chain_tag * kymera_chain_handle_t;

/*! A structure to define a downloadable capability bundle.
    Each bundle has a file name, and contains a number of downloadable operators */
typedef struct
{
    /*! The file name of the bundle */
    const char* file_name;
    /*! The processors for which the downloadable capabilities are to be made available.
        The bundle may be made available on audio processor 0 or both audio processor. */
    capability_bundle_processor_availability_t processors;
} capability_bundle_t;

/*! A structure to define the configuration of a group of bundles */
typedef struct
{
    /*! Pointer to an array of capability bundles */
    const capability_bundle_t *capability_bundles;
    /*! The number of capability bundles in the array */
    unsigned number_of_capability_bundles;
} capability_bundle_config_t;

/*! \deprecated Helper macro to initialise chain_config_t structure with values by
    defining the chain inputs, outputs and internal connections.
 */ 
#define MAKE_CHAIN_CONFIG(id, ucid, operators, inputs, outputs, connections) { \
    (id), \
    (ucid), \
    (operators), ARRAY_DIM((operators)), \
    (inputs), ARRAY_DIM((inputs)), \
    (outputs), ARRAY_DIM((outputs)), \
    (connections), ARRAY_DIM((connections)), \
    NULL, 0}

/*! \deprecated Helper macro to initialise chain_config_t structure with values by
    defining the chain inputs and outputs only. 
 */ 
#define MAKE_CHAIN_CONFIG_NO_CONNECTIONS(id, ucid, operators, inputs, outputs) { \
    (id), \
    (ucid), \
    (operators), ARRAY_DIM((operators)), \
    (inputs), ARRAY_DIM((inputs)), \
    (outputs), ARRAY_DIM((outputs)), \
    NULL, 0, \
    NULL, 0}

/*! \deprecated Helper macro to initialise chain_config_t structure with values by
    defining the chain outputs only. 
 */ 
#define MAKE_CHAIN_CONFIG_NO_INPUTS_NO_CONNECTIONS(id, ucid, operators, outputs) { \
    (id), \
    (ucid), \
    (operators), ARRAY_DIM((operators)), \
    NULL, 0, \
    (outputs), ARRAY_DIM((outputs)), \
    NULL, 0, \
    NULL, 0}

/*! Helper macro to initialise chain_config_t structure with values by
    defining data paths through the chain.
 */ 
#define MAKE_CHAIN_CONFIG_WITH_PATHS(id, ucid, operators, paths) { \
    (id), \
    (ucid), \
    (operators), ARRAY_DIM((operators)), \
    NULL, 0, \
    NULL, 0, \
    NULL, 0, \
    paths, ARRAY_DIM((paths))}

/*! \deprecated Helper macro to initialise chain_config_t structure with values by
    defining the chain inputs and internal connections only. 
 */ 
#define MAKE_CHAIN_CONFIG_NO_OUTPUTS(id, ucid, operators, inputs, connections) { \
    (id), \
    (ucid), \
    (operators), ARRAY_DIM((operators)), \
    (inputs), ARRAY_DIM((inputs)), \
    NULL, 0, \
    (connections), ARRAY_DIM((connections)), \
    NULL, 0}

/*! Shorthand for "no set-up needed for this operator" */
#define NO_OPERATOR_SETUP {0, NULL}

/*! Helper macro to simplify definition of operator set-up values */
#define MAKE_OPERATOR_SETUP(setup_items) { ARRAY_DIM((setup_items)), setup_items }

/*! Helper macro to instantiate an operator on Audio processo 0 with default priority and no set-up */
#define MAKE_OPERATOR_CONFIG(cap_id, role) { (cap_id), (role), OPERATOR_PROCESSOR_ID_0, DEFAULT_OPERATOR_PRIORITY, NO_OPERATOR_SETUP }

/*! Same as MAKE_OPERATOR_CONFIG, but on Audio processor 1 */
#define MAKE_OPERATOR_CONFIG_P1(cap_id, role) { (cap_id), (role), OPERATOR_PROCESSOR_ID_1, DEFAULT_OPERATOR_PRIORITY, NO_OPERATOR_SETUP }

/*! Same as MAKE_OPERATOR_CONFIG, but with high priority */
#define MAKE_OPERATOR_CONFIG_PRIORITY_HIGH(cap_id, role) { (cap_id), (role), OPERATOR_PROCESSOR_ID_0, operator_priority_high, NO_OPERATOR_SETUP }

/*! Same as MAKE_OPERATOR_CONFIG, but with medium priority */
#define MAKE_OPERATOR_CONFIG_PRIORITY_MEDIUM(cap_id, role) { (cap_id), (role), OPERATOR_PROCESSOR_ID_0, operator_priority_medium, NO_OPERATOR_SETUP }

/*! Same as MAKE_OPERATOR_CONFIG, but with low priority */
#define MAKE_OPERATOR_CONFIG_PRIORITY_LOW(cap_id, role) { (cap_id), (role), OPERATOR_PROCESSOR_ID_0, operator_priority_low, NO_OPERATOR_SETUP }

/*! Same as MAKE_OPERATOR_CONFIG, but with lowest priority */
#define MAKE_OPERATOR_CONFIG_PRIORITY_LOWEST(cap_id, role) { (cap_id), (role), OPERATOR_PROCESSOR_ID_0, operator_priority_lowest, NO_OPERATOR_SETUP }

/*! Same as MAKE_OPERATOR_CONFIG, but with set-up */
#define MAKE_OPERATOR_CONFIG_WITH_SETUP(cap_id, role, setup_items) { (cap_id), (role), OPERATOR_PROCESSOR_ID_0, DEFAULT_OPERATOR_PRIORITY, MAKE_OPERATOR_SETUP(setup_items) }

/*! Same as MAKE_OPERATOR_CONFIG, but on Audio processor 1 and with set-up */
#define MAKE_OPERATOR_CONFIG_P1_WITH_SETUP(cap_id, role, setup_items) { (cap_id), (role), OPERATOR_PROCESSOR_ID_1, DEFAULT_OPERATOR_PRIORITY, MAKE_OPERATOR_SETUP(setup_items) }

/*! Same as MAKE_OPERATOR_CONFIG, but with low priority and set-up */
#define MAKE_OPERATOR_CONFIG_PRIORITY_LOW_WITH_SETUP(cap_id, role, setup_items) { (cap_id), (role), OPERATOR_PROCESSOR_ID_0, operator_priority_low, MAKE_OPERATOR_SETUP(setup_items) }

/*! Same as MAKE_OPERATOR_CONFIG, but with medium priority and set-up */
#define MAKE_OPERATOR_CONFIG_PRIORITY_MEDIUM_WITH_SETUP(cap_id, role, setup_items) { (cap_id), (role), OPERATOR_PROCESSOR_ID_0, operator_priority_medium, MAKE_OPERATOR_SETUP(setup_items) }

/*! Same as MAKE_OPERATOR_CONFIG, but with high priority and set-up */
#define MAKE_OPERATOR_CONFIG_PRIORITY_HIGH_WITH_SETUP(cap_id, role, setup_items) { (cap_id), (role), OPERATOR_PROCESSOR_ID_0, operator_priority_high, MAKE_OPERATOR_SETUP(setup_items) }

/*! \brief Specify files which contain downloadable capability bundles

An example configuration:

    const capability_bundle_t bundles[] = {{"bundle_file_1.dkcs",
                                            capability_bundle_available_p0};
                                           {"bundle_file_2.dkcs",
                                            capability_bundle_available_p0}};
    const capability_bundle_config_t bundle_config = {bundles, ARRAY_DIM(bundles)};

The chain library just saves the pointer passed to this function, it does not 
copy the configuration. The caller must ensure the configuration is valid while 
it is in use by the chain library.

Calling this function with a NULL argument clears the configuration.
*/
void ChainSetDownloadableCapabilityBundleConfig(const capability_bundle_config_t *config);

/*! \brief Create a chain as defined by the config.

The caller must ensure that the chain_config_t structure pointed to by config, 
exists until the chain is destroyed using ChainDestroy().

The chain_config_t should use the paths parameter to set up the chain in new code.
The deprecated parameters inputs, outputs and connections may be omitted when a
valid paths configuration is provided.

Each path entry has an associated role which is used to identify the inputs and
outputs of the chain. When ChainConnect() is called the operator terminals (nodes) 
in each path are connected in the order in which they are listed in the array, i.e. 
<em> nodes[0].output_terminal</em> to <em> nodes[1].input_terminal</em>,
<em> nodes[1].output_terminal</em> to <em> nodes[2].input_terminal</em> and so on.
*/
kymera_chain_handle_t ChainCreate(const chain_config_t *config);

/*! \brief Create a chain as defined by the config with some operators replaced
or removed. 

Same as ChainCreate(const chain_config_t *config) but with the option to replace 
or remove some operators using a filter.

The filter consists of an array of operator_config_t configurations. 

Please refer to the documentation section on \ref filtering for a more detailed 
description. 

*/
kymera_chain_handle_t ChainCreateWithFilter(const chain_config_t *config, const operator_filters_t* filter);

/*! \brief Destroy a chain.
*/
void ChainDestroy(kymera_chain_handle_t handle);

/*! \brief Retrieve an operator by its role.
*/
Operator ChainGetOperatorByRole(const kymera_chain_handle_t handle, unsigned operator_role);

/*! \brief Get the chain input.

Returns a Sink which represents the chain input for a given path role.

*/
Sink ChainGetInput(kymera_chain_handle_t handle, unsigned input_role);

/*! \brief Get the chain output.

Returns a Source which represents the chain ouput for a given path role.

*/
Source ChainGetOutput(kymera_chain_handle_t handle, unsigned output_role);

/*! \brief Make connections between operators as defined by the config supplied
to ChainCreate().
*/
void ChainConnect(kymera_chain_handle_t handle);

/*! \brief Make connections between operators as defined by the path associated
with path_role in the config supplied to ChainCreate().
*/
void ChainConnectWithPath(kymera_chain_handle_t handle, unsigned path_role);

/*! \brief Connect a source to a chain input.

Returns TRUE on success and FALSE on failure.

Equivalent to 
   
    Sink sink = ChainGetInput(handle, input_role);
    StreamConnect(source, sink);
*/
bool ChainConnectInput(kymera_chain_handle_t handle, Source source, unsigned input_role);

/*! \brief Connect a chain output to a sink.

Returns TRUE on success and FALSE on failure.

Equivalent to 
   
    Source source = ChainGetOutput(handle, output_role);
    StreamConnect(source, sink);
*/
bool ChainConnectOutput(kymera_chain_handle_t handle, Sink sink, unsigned output_role);

/*! \brief Start a chain.

On failure to start the chain this function will Panic
*/
void ChainStart(kymera_chain_handle_t handle);

/*! \brief Start a chain.

Returns TRUE on success and FALSE on failure

On failure to start the chain this function will stop all operators in the chain
*/
bool ChainStartAttempt(kymera_chain_handle_t handle);

/*! \brief Stop a chain.
*/
void ChainStop(kymera_chain_handle_t handle);

/*! \brief Connect the outputs of one chain to the inputs of another.
*/
void ChainJoin(kymera_chain_handle_t source_chain, kymera_chain_handle_t sink_chain, unsigned count, const chain_join_roles_t *connect_list);

/*******************************************************************************
DESCRIPTION
    Connect the outputs of one chain to the inputs of another only if roles matched, and returns number of successful connections made.
*/
unsigned ChainJoinMatchingRoles(kymera_chain_handle_t source_handle, kymera_chain_handle_t sink_handle, unsigned count);


/*! \brief Configure chain operators with the messages. 

The messages are not part of the configuration as the required messages are 
likely to change dynamically.
*/
void ChainConfigure(kymera_chain_handle_t handle, const chain_operator_message_t *messages,  unsigned number_of_messages);

/*! \brief Set sample rate of a chain

Set sample rate of all operators with buffer allocation for all operators except for those on the excluded list.
*/
void ChainConfigureSampleRate(kymera_chain_handle_t handle, uint32 sample_rate, const unsigned *excluded_roles, unsigned excluded_roles_count);

/*! \brief Set sample rate of a chain without allocating the buffer.

Set sample rate of each operator for all operators except for those on the excluded list.
*/
void ChainConfigureSampleRateWithoutAllocatingBuffer(kymera_chain_handle_t handle, uint32 sample_rate, const unsigned *excluded_roles, unsigned excluded_roles_count);

/*! \brief Find a chain by the specified ID and index.

This is only intended for unit test and should not be used in application code.
*/
kymera_chain_handle_t ChainFindById(chain_id_t id, unsigned index);

/*! \brief Get the list of operators within a chain 

Returns a pointer to an array of Operator elements. The number of elements contained 
in the array is written into *count. 

This is only intended for unit test and should not be used in application code.
*/
const Operator *ChainGetOperatorList(kymera_chain_handle_t handle, unsigned *count);

/*! \brief Returns pointer to the chain chain_config_t structure

Returns pointer to the chain_config_t structure supplied to ChainCreate() or
ChainCreateWithFilter() when the chain was created.

This is only intended for unit test and should not be used in application code.
*/
const chain_config_t *ChainGetConfig(kymera_chain_handle_t handle);

/*! \brief Returns audio use case ID for a given chain configuration

Returns the audio_ucid memeber from the chain_config_t structure.

This is only intended for unit test and should not be used in application code.
*/
audio_ucid_t ChainGetUseCase(const chain_config_t *config);

/*! \brief Transitions the Audio subsystem to low power mode

Transitions the Audio subsystem to low power mode. Note that this API
will only work correctly if at least one operator has been configured to
run in low power mode (e.g. through OperatorFrameworkTriggerNotificationStart) 
*/
void ChainSleep(const kymera_chain_handle_t chain);

/*! \brief Wakes the Audio subsystem from low power mode

Transitions the Audio subsystem to full power mode.
*/
void ChainWake(const kymera_chain_handle_t chain);

/*! \brief Reset any static variables during testing.

This is only intended for unit test and should not be used in application code.
*/
void ChainTestReset(void);

#endif /* LIBS_CHAIN_CHAIN_H_ */
