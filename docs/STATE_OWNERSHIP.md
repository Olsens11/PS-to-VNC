# PS-to-VNC State Ownership

## Rule

Every important mutable state group has one owning subsystem.

Other subsystems access that state through the owner's API or through a
deliberately passed value.

Application-wide writable `extern` variables are transitional.

## Target ownership groups

### app/startup

Owns:

- startup display transaction lifecycle;
- authoritative startup display profile;
- confirmed startup display profile;
- startup reconciliation requirements.

Migration seeds include the startup transaction/profile variables currently
provided by `ps2ip.o`.

### app/display_transition

Owns product-level temporary display transition state:

- requested temporary mode;
- baseline mode;
- transition lifecycle;
- lease/timeout orchestration;
- rollback progress;
- transition error result;
- confirmation/restore workflow.

The local active hardware mode itself remains video-owned.

### app/recovery

Owns recovery-policy state such as manual refresh/cooldown lifecycle where
that state represents product policy rather than protocol mechanism.

### rfb/session

Owns:

- RFB connection state;
- receive buffer/cursors;
- exact transport/session framing state;
- outbound RFB queue;
- negotiated encoding;
- live message/rectangle decode state;
- connection generation where applicable.

### rfb/desktop_resize

Owns ExtendedDesktopSize negotiation state:

- expected resize;
- observed server desktop description;
- first-screen fields;
- resize result/reason.

### video/mode

Owns:

- active video mode;
- active display backend;
- supported mode catalog.

### video/geometry

Owns or computes:

- logical desktop dimensions;
- presented/output geometry;
- geometry lock/invariants.

Persistent values should live in an appropriate video state structure;
pure derived values should be calculated rather than duplicated.

### video/framebuffer

Owns:

- framebuffer storage;
- high-resolution source/presentation buffers;
- framebuffer validity/staleness state.

### video/mapping

Owns:

- high-resolution mapping cache;
- mapping validity;
- sample/edge tables.

The current static mapping cache symbols in the framebuffer-tail object are
natural seeds for this module.

### video/calibration

Owns:

- safe-area width/height;
- safe-area offsets;
- full-bypass state;
- calibration interaction state.

### input/controller

Owns:

- controller thread/polling state;
- pad state;
- controller pause/ack lifecycle.

### input/bindings

Owns:

- hotkey binding definitions;
- hold/chord recognition state;
- binding-trigger state.

The long-term configuration representation of bindings is owned by
`config`; the live recognition state is owned by `input`.

### ui/display_menu

Owns display-menu navigation/render state.

### ui/system_menu

Owns system-menu state and countdown presentation state.

### ui/osk

Owns:

- OSK visibility;
- row/column/page;
- modifier state;
- render generation/pixel backing specific to the OSK.

### management/client

Owns management transport/client-local state only.

### management/display_policy

Owns the most recently retrieved remote policy representation where caching
is necessary.

It does not own UI selection state.

### diagnostics/debug

Owns debug-stage and UDP diagnostic state.

### diagnostics/profiling

Owns profiling counters/timestamps.

## Top-level composition

The application may compose subsystem state in one top-level structure for
lifetime management.

That does not grant every module permission to reach into every field.

Each subsystem API receives only the context/value it legitimately needs.

## Migration rule

When a state group is normalized:

1. choose the final owner first;
2. move the state and the invariants that govern it together;
3. replace foreign direct writes with owner API calls;
4. replace foreign direct reads with explicit queries or passed values;
5. verify dependency direction;
6. only then delete the corresponding compatibility extern declarations.

A lower raw global count is not itself proof of better architecture.
