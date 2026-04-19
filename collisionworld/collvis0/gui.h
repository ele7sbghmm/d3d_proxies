#pragma once

struct CollVisContext;

class Gui {
public:
	auto begin(CollVisContext& ctx);
	auto new_frame(CollVisContext& ctx) -> void;
};
