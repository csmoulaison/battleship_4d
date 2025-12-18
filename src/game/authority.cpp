/*
Authority: The authoritative arbitrator of game state.

The data within the authority is accessed/modified by an outer layer in 
accordance with the network configuration.
- For example, the packet handler in a server handles packets from the server
  side and modifies authority data accordingly. If it receives an action packet
  from a remote client, for instance, it would add that input to the agent which
  would then be processed by the Authority. The authority would recognize and 
  process it the same way if it was instead populated by a local client or CPU.
- The general pattern here is to encode all events and actions into the data of 
  the session or agents themselves. If the server would not be able to infer the
  temporality of a particular event, it must be encoded.
*/

#define AUTHORITY_MAX_AGENTS 8

struct Authority {
	Session session;
	Agent agents[AUTHORITY_MAX_AGENTS];
	i32 agents_len;
};

void authority_update(Authority* authority) 
{
	if(authority->agents_len < 2) {
		// NOW: We need to end the game here. I suppose this might want to be done on
		// the outside?
	}

	Agent* agent = &authority->agents[turn % authority->agents_len];
	if(agent->action.turn == authority->session.turn) {
		// NOW: We have a new action by the agent who's turn it is.
		// Resolve it and iterate the turn.
		authority->session.turn++;
	}
}
