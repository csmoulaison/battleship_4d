enum class ActionType { Move, Query, Fire };
enum class Axis { XY, XZ, YZ };
enum class Direction { Left, Right, Up, Down, Forward, Back };

struct Action {
	ActionType type;
	i32 turn;
	union {
		struct move { Direction direction; };
		struct query { Axis axis; u8 position; };
		struct fire { u32 position; };
	};
};
