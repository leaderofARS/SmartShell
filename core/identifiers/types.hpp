// SmartShell — Core: Project-Wide Identifier Types
// Each identifier domain gets its own distinct type via StrongId.
#pragma once

#include "id.hpp"

namespace ss {

// Tags — empty structs used only as template parameters
struct ProcessIdTag   {};
struct SessionIdTag   {};
struct TabIdTag       {};
struct PaneIdTag      {};
struct JobIdTag       {};
struct CommandIdTag   {};
struct ThreadIdTag    {};

// Public identifier types
using ProcessId  = StrongId<ProcessIdTag,  u32>;
using SessionId  = StrongId<SessionIdTag,  u32>;
using TabId      = StrongId<TabIdTag,      u32>;
using PaneId     = StrongId<PaneIdTag,     u32>;
using JobId      = StrongId<JobIdTag,      u32>;
using CommandId  = StrongId<CommandIdTag,  u64>;
using ThreadId   = StrongId<ThreadIdTag,   u32>;

} // namespace ss
