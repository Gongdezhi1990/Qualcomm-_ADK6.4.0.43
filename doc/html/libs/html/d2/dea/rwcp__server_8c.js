var rwcp__server_8c =
[
    [ "SERVER_DATA_T", "dd/da2/structSERVER__DATA__T.html", "dd/da2/structSERVER__DATA__T" ],
    [ "RWCP_COMMAND_MASK", "d2/dea/rwcp__server_8c.html#a6853cd4b19dd1390af878700a53fc2ec", null ],
    [ "RWCP_HEADER_OFFSET", "d2/dea/rwcp__server_8c.html#a8458c0229a8b5d7118642a2b82e9cb80", null ],
    [ "RWCP_PAYLOAD_OFFSET", "d2/dea/rwcp__server_8c.html#a760a833b3f20efe1c8a08a2e0290cf2b", null ],
    [ "RWCP_RECEIVE_WINDOW_MAX", "d2/dea/rwcp__server_8c.html#aa30e9fae179dc6d1e2b921833eb71358", null ],
    [ "RWCP_SEQUENCE_MASK", "d2/dea/rwcp__server_8c.html#a5885eab4d247799e93668a42ae6716ea", null ],
    [ "RWCP_SEQUENCE_NUMBER_INVALID", "d2/dea/rwcp__server_8c.html#a608d1ecf2be7d6f1eaa3119a61eaebc1", null ],
    [ "RWCP_SEQUENCE_NUMBER_MAX", "d2/dea/rwcp__server_8c.html#a1f83998f96c343813aeec46a22e7e01f", null ],
    [ "RWCP_SERVER_DEBUG", "d2/dea/rwcp__server_8c.html#a37e438c66bd292fe3b287d6e9d944592", null ],
    [ "rwcp_client_commands_t", "d2/dea/rwcp__server_8c.html#aeb508993e43edd62910fbf59689f3d91", [
      [ "RWCP_CLIENT_CMD_DATA", "d2/dea/rwcp__server_8c.html#aeb508993e43edd62910fbf59689f3d91a39d038192be45848a79065167c0ac6e4", null ],
      [ "RWCP_CLIENT_CMD_SYN", "d2/dea/rwcp__server_8c.html#aeb508993e43edd62910fbf59689f3d91a0e900a74b50588312a1a233e8784b119", null ],
      [ "RWCP_CLIENT_CMD_RST", "d2/dea/rwcp__server_8c.html#aeb508993e43edd62910fbf59689f3d91a8f2c20d15d125d7427a2751cce614ec2", null ],
      [ "RWCP_CLIENT_CMD_RESERVED", "d2/dea/rwcp__server_8c.html#aeb508993e43edd62910fbf59689f3d91a742abc8c88914770dc34fdfc3ef68ec6", null ]
    ] ],
    [ "rwcp_data_pkts_t", "d2/dea/rwcp__server_8c.html#a209b19e8e5822ad2707aec29c303cc71", [
      [ "RWCP_DATA_PKT_IN_SEQUENCE", "d2/dea/rwcp__server_8c.html#a209b19e8e5822ad2707aec29c303cc71a0813b31063145234f789fb8840380a21", null ],
      [ "RWCP_DATA_PKT_DUPLICATE", "d2/dea/rwcp__server_8c.html#a209b19e8e5822ad2707aec29c303cc71a58746649b3e40170138ec9c0930fe89f", null ],
      [ "RWCP_DATA_PKT_OUT_OF_SEQUENCE", "d2/dea/rwcp__server_8c.html#a209b19e8e5822ad2707aec29c303cc71a6c3bdafb05584f1f80560f4921c9d3d3", null ],
      [ "RWCP_DATA_PKT_DISCARDED", "d2/dea/rwcp__server_8c.html#a209b19e8e5822ad2707aec29c303cc71aea6bb22a96c0e44da6c847c1956359c1", null ]
    ] ],
    [ "rwcp_server_commands_t", "d2/dea/rwcp__server_8c.html#a2d1c39daeaab8bc36ebc47583e1893c4", [
      [ "RWCP_SERVER_CMD_DATA_ACK", "d2/dea/rwcp__server_8c.html#a2d1c39daeaab8bc36ebc47583e1893c4ab4ddb75e8dca4e1b3270c3e995b839ed", null ],
      [ "RWCP_SERVER_CMD_SYN_ACK", "d2/dea/rwcp__server_8c.html#a2d1c39daeaab8bc36ebc47583e1893c4ac6011f5bd22d549bc5f55dee218cd926", null ],
      [ "RWCP_SERVER_CMD_RST", "d2/dea/rwcp__server_8c.html#a2d1c39daeaab8bc36ebc47583e1893c4a8ae1c93deef063ad067904feca92df77", null ],
      [ "RWCP_SERVER_CMD_GAP", "d2/dea/rwcp__server_8c.html#a2d1c39daeaab8bc36ebc47583e1893c4a95d05e6b9752e50589a78150e36d7466", null ]
    ] ],
    [ "handleDataSegment", "d2/dea/rwcp__server_8c.html#a1c0ad56eaa079d7852533f944ae07732", null ],
    [ "isNextSequence", "d2/dea/rwcp__server_8c.html#ad6788b28b6e882b15b3d85bb044ff512", null ],
    [ "isOutOfSequence", "d2/dea/rwcp__server_8c.html#a9e5b0c2f11bab78802a8b94df4ec6f58", null ],
    [ "isRWwcpControlCmd", "d2/dea/rwcp__server_8c.html#a4ec387f8cf616cab1d2b87b8e0b2114d", null ],
    [ "nextExpectedSequenceNumber", "d2/dea/rwcp__server_8c.html#a31ae04f3b81c1f8dfa0760ae0d71f241", null ],
    [ "rwcpDataAck", "d2/dea/rwcp__server_8c.html#af54deffe0f35dadf66967279ec115d38", null ],
    [ "rwcpGap", "d2/dea/rwcp__server_8c.html#a24a4adc7018dcacabf8f8bc4203e7347", null ],
    [ "RwcpGetOutOfSequenceStatus", "d2/dea/rwcp__server_8c.html#a9e449bde3d593cd0ac115e653e27cfbc", null ],
    [ "RwcpGetProtocolState", "d2/dea/rwcp__server_8c.html#a126e54b90316061f4795d3bd727a76b6", null ],
    [ "rwcpRst", "d2/dea/rwcp__server_8c.html#a37cbc3064e8a7b0e2fd749155bac8da7", null ],
    [ "rwcpRstAck", "d2/dea/rwcp__server_8c.html#a10b1684edf81bd23889cbdbe55e4c0e2", null ],
    [ "rwcpSendNotification", "d2/dea/rwcp__server_8c.html#a1e85f67762232e734e27f8949b967bf9", null ],
    [ "RwcpServerFlowControl", "d2/dea/rwcp__server_8c.html#a91d6c01840526346c332cdf0bf7e2f22", null ],
    [ "RwcpServerHandleMessage", "d2/dea/rwcp__server_8c.html#a529f23cba2004426e10bc8603108ce56", null ],
    [ "RwcpServerInit", "d2/dea/rwcp__server_8c.html#a8bc613c08f19ab2623f911a8897da9a1", null ],
    [ "RwcpSetClientTask", "d2/dea/rwcp__server_8c.html#a1d26b4887dfb8e6eaa28fb76cdc6eac0", null ],
    [ "rwcpSynAck", "d2/dea/rwcp__server_8c.html#a7f41d2a044c48cea7980252e2c0ee509", null ],
    [ "g_server_data", "d2/dea/rwcp__server_8c.html#a1c2f2b2cd0ef41fd73631c0e792a27e6", null ]
];