# Media-object composition notes

Status: **design concept / future hybrid RFB + MPEG work; not implemented**.

This note records the media-region model developed during the H1 resident media
harness experiments on 2026-09-08. It is intentionally subordinate to the
project's clean architecture and current issue sequencing. It does not begin
Issue #40 and does not make hybrid MPEG/RFB behavior part of the current
qualified product.

## Experimental result that motivated this model

H1 proved that the Pi can capture a selected desktop region, MPEG-2 encode that
region, and ask the PS2 to present the result at the corresponding desktop
location.

A manual YouTube-player experiment converged on an approximately grid-aligned
rectangle of:

    x=16
    y=112
    width=432
    height=240

That rectangle was useful for the current H1 codec experiment, but the product
model must not assume that a media area itself is aligned to 16-pixel MPEG
macroblock boundaries.

The current Pi path captures the complete active X11 desktop and crops inside
FFmpeg before MPEG encoding. A later implementation may capture only the
selected source region directly after its geometry is known.

## Core design principle

**A media region is a semantic desktop object, not a codec rectangle.**

The desktop/application owns exact pixel geometry. MPEG alignment, padding and
storage geometry are implementation details beneath that semantic object.

Keep these coordinate/geometry concepts separate:

1. **Desktop geometry** -- the active remote desktop coordinate space.
2. **Host-window geometry** -- the exact X11 window that contains or represents
   the media presentation.
3. **Media-object geometry** -- the exact visible media bounds, preferably
   stored relative to the host window as well as resolvable to desktop
   coordinates.
4. **Codec surface geometry** -- any padded/aligned width and height required by
   MPEG-2/IPU/GS mechanisms.

For example, a real media area may be:

    semantic media bounds: x=10 y=107 width=443 height=248

while an internal codec surface may be:

    codec surface: width=448 height=256

The extra pixels are padding/storage, not permission to capture neighboring
browser chrome or to alter the object's semantic location.

## Host-window attachment

When possible, a detected media object should attach to the X11 object beneath
it rather than remaining an absolute desktop rectangle.

Example:

    Desktop
      -> browser/media-player window
          -> media object at host-relative x/y/width/height

If the host window moves, the media object follows by recomputing desktop
coordinates from the unchanged host-relative geometry:

    media_desktop_x = host_desktop_x + media_relative_x
    media_desktop_y = host_desktop_y + media_relative_y

A simple move therefore does not require media rediscovery.

If the host resizes, stored normalized geometry may be used as an initial
prediction, after which activity evidence can refine/reacquire the true media
bounds.

A dedicated video/child window is the strongest case: its exact X11 geometry
may itself supply the media bounds. Otherwise the system should detect a media
region inside a more general host window such as Chromium.

## Generic media discovery rather than an application whitelist

The design should not depend on knowing that an application is Chromium, VLC,
mpv, or any other named player.

The relevant questions are:

- which window owns this region? and
- which stable rectangular/near-rectangular region behaves like continuously
  changing video?

Useful evidence can include:

- X11 top-level and child-window geometry;
- X11 move/resize/lifetime notifications;
- X11 damage/change information where practical;
- individual RFB update rectangles before they are conservatively unioned into
  the framebuffer's single dirty bounding rectangle; and
- temporal pixel/tile activity.

The through-Issue-39 framebuffer's conservative `dirty_rect` alone is not
sufficient for discovery because separate changes can be unioned into a large
bounding box that includes untouched desktop area.

A generic detector can project individual change rectangles onto a rolling
activity map (a 16x16 tile grid is convenient for analysis, but must not define
semantic object geometry). Over roughly seconds rather than one update, it can
look for a large contiguous region with sustained dense changes and relatively
stable outer bounds.

This naturally distinguishes likely video from common UI activity:

- cursor movement is small and transient;
- a caret is tiny and periodic;
- a progress bar is narrow;
- scrolling may dirty a large area briefly but normally does not remain a stable
  dense region; and
- video tends to produce sustained change inside a stable presentation area.

Detection must have memory/hysteresis. A black scene, static title card,
subtitle transition, mouse crossing, or brief application redraw must not make
the media object jump or disappear immediately.

A useful progression is:

1. detect and report a candidate media region only;
2. compare it to visible ground truth;
3. attach the candidate to its host window;
4. predict/follow host movement without rediscovery;
5. reacquire/refine after meaningful resize or sustained contradictory
   evidence; and
6. only after confidence is proven, allow the detector to drive MPEG capture
   and presentation automatically.

## Exact media bounds versus MPEG macroblocks

Do not snap the media object's true bounds to a 16x16 grid.

If the codec path requires macroblock-aligned storage, preserve the exact media
bounds and adapt below them. For example:

    exact source region: 443x248
    internal encoded surface: 448x256

The adapter may pad the right/bottom edges (black, replicated edge pixels, or
another proven mechanism) without capturing unrelated neighboring desktop
pixels.

The PS2 compositor should likewise distinguish the encoded texture/storage
surface from the exact visible media bounds. If the decoder exposes padded
pixels, texture coordinates or another presentation mechanism should prevent
those padding pixels from becoming part of the semantic desktop object.

Movement and placement remain pixel-exact even when storage dimensions are
aligned internally.

## Optional visibility/shape mask

A rectangular bounding box may not describe the exact visible media shape.
Modern players can use rounded corners or other clipping.

Therefore a future media object may carry an optional visibility/shape mask in
addition to exact x/y/width/height.

Conceptually:

    media object
      - exact host-relative bounds
      - exact desktop-resolved bounds
      - optional visibility/shape mask
      - detection confidence/activity history
      - host-window attachment

The MPEG coding surface remains rectangular. The compositor decides where that
surface is visible.

For rounded-corner video, the RFB desktop should remain visible in the clipped
corner pixels instead of forcing the MPEG rectangle inward and losing real
video along all four edges.

The mask may eventually be learned from temporal activity: pixels/tiles inside
moving video remain active while stable browser background in clipped corners
remains inactive. Such inference must use temporal evidence so dark scenes or
letterboxing are not mistaken for clipping.

The PS2 presentation implementation may use a true mask/alpha mechanism or a
simpler geometry approximation such as a small set of horizontal strips for
rounded corners. The semantic model should not depend on which GS mechanism is
chosen.

## Future compositor ownership

When hybrid desktop/media presentation is implemented, there should be one
presentation owner with explicit layer order:

    RFB desktop
        -> MPEG media object(s), clipped to semantic bounds/mask
            -> local cursor / OSK / local UI

RFB decoding should continue underneath an active MPEG region even while RFB
presentation there is suppressed. That preserves the authoritative remote
framebuffer so the desktop is immediately current when MPEG ownership is
removed; it avoids requiring a full refresh merely because a media overlay
ended.

The RFB stream should use its already-reserved logical channel on the same PSTV
physical connection. Hybrid composition must not regress to competing
PS2-facing TCP streams.

## Separation of detection, attachment, codec and presentation

Keep the future implementation modular:

- **detector**: identifies likely dynamic/media regions from generic evidence;
- **host attachment**: binds a candidate to an X11 window and tracks movement,
  resize and lifetime;
- **semantic media object**: owns exact geometry/mask/confidence independent of
  codec constraints;
- **capture/codec adapter**: converts exact source geometry into any padded MPEG
  surface needed by the encoder/decoder;
- **compositor**: resolves exact desktop placement and layer/mask behavior; and
- **RFB authority**: continues decoding the complete desktop independently of
  whether part of it is currently hidden beneath MPEG presentation.

This separation is intended to let an unknown future media application work
without application-specific code while keeping codec/hardware constraints out
of desktop semantics.
