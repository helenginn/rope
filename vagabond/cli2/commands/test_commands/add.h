#include "../../runtime/CommandSpec.h"

int add(int left, int right);

namespace rope::cli::commands
{
	inline constexpr auto AddMeta = command_meta{
	    .name = "add",
	    .description = "Adds two integers",
	    .handler = &add,
	};

	inline constexpr auto AddPos1Meta = argument_meta{
	    .name = "left",
	    .description = "The left value for addition",
	};
	using AddPositional1 = positional<int, AddPos1Meta>;

	inline constexpr auto AddPos2Meta = argument_meta{
	    .name = "right",
	    .description = "The right value for addition",
	};
	using AddPositional2 = positional<int, AddPos2Meta>;

	using Add = command<AddMeta, AddPositional1, AddPositional2>;

} // namespace rope::cli::commands
