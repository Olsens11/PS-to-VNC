# Clean symbols — `src/framebuffer`

DIRECTORY=src/framebuffer
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=COMPLETE

This directory owns the authoritative CPU-side remote desktop image and its bounded framebuffer storage/update mechanisms.

The inventory below covers clean-generation symbols defined directly in this directory.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| rect_fits | function | src/framebuffer/framebuffer.c | desktop framebuffer | file | Validates a nonempty rectangle using widened arithmetic before any pixel write. | authoritative framebuffer semantics |
| framebuffer | parameter | src/framebuffer/framebuffer.c | rect_fits | local | Supplies geometry against which rectangle bounds are validated. | authoritative framebuffer semantics |
| x | parameter | src/framebuffer/framebuffer.c | rect_fits | local | Gives the rectangle left coordinate. | framebuffer rectangle bounds |
| y | parameter | src/framebuffer/framebuffer.c | rect_fits | local | Gives the rectangle top coordinate. | framebuffer rectangle bounds |
| width | parameter | src/framebuffer/framebuffer.c | rect_fits | local | Gives the nonzero rectangle width to validate. | framebuffer rectangle bounds |
| height | parameter | src/framebuffer/framebuffer.c | rect_fits | local | Gives the nonzero rectangle height to validate. | framebuffer rectangle bounds |
| right | variable | src/framebuffer/framebuffer.c | rect_fits | local | Stores the widened exclusive right edge used to prevent 16-bit wraparound. | framebuffer rectangle bounds |
| bottom | variable | src/framebuffer/framebuffer.c | rect_fits | local | Stores the widened exclusive bottom edge used to prevent 16-bit wraparound. | framebuffer rectangle bounds |
| include_dirty | function | src/framebuffer/framebuffer.c | desktop framebuffer | file | Expands the conservative dirty bounding rectangle around one successful write. | authoritative framebuffer semantics |
| framebuffer | parameter | src/framebuffer/framebuffer.c | include_dirty | local | Supplies mutable dirty-state ownership for the authoritative image. | framebuffer dirty state |
| x | parameter | src/framebuffer/framebuffer.c | include_dirty | local | Gives the newly changed rectangle left coordinate. | framebuffer dirty state |
| y | parameter | src/framebuffer/framebuffer.c | include_dirty | local | Gives the newly changed rectangle top coordinate. | framebuffer dirty state |
| width | parameter | src/framebuffer/framebuffer.c | include_dirty | local | Gives the newly changed rectangle width. | framebuffer dirty state |
| height | parameter | src/framebuffer/framebuffer.c | include_dirty | local | Gives the newly changed rectangle height. | framebuffer dirty state |
| left | variable | src/framebuffer/framebuffer.c | include_dirty | local | Stores the merged dirty rectangle left edge. | framebuffer dirty state |
| top | variable | src/framebuffer/framebuffer.c | include_dirty | local | Stores the merged dirty rectangle top edge. | framebuffer dirty state |
| right | variable | src/framebuffer/framebuffer.c | include_dirty | local | Stores the merged dirty rectangle exclusive right edge. | framebuffer dirty state |
| bottom | variable | src/framebuffer/framebuffer.c | include_dirty | local | Stores the merged dirty rectangle exclusive bottom edge. | framebuffer dirty state |
| current_right | variable | src/framebuffer/framebuffer.c | include_dirty | local | Stores the prior dirty rectangle exclusive right edge during union. | framebuffer dirty state |
| current_bottom | variable | src/framebuffer/framebuffer.c | include_dirty | local | Stores the prior dirty rectangle exclusive bottom edge during union. | framebuffer dirty state |
| pstvnc_framebuffer_init | function | src/framebuffer/framebuffer.c | desktop framebuffer | public | Initializes framebuffer metadata around caller-owned pixel storage. | ISSUE7_MINIMAL_CORE: Owned authoritative framebuffer |
| framebuffer | parameter | src/framebuffer/framebuffer.c | pstvnc_framebuffer_init | local | Supplies the framebuffer value whose metadata is initialized. | desktop framebuffer lifecycle |
| pixels | parameter | src/framebuffer/framebuffer.c | pstvnc_framebuffer_init | local | Supplies caller-owned CPU pixel storage without asserting content authority. | desktop framebuffer lifecycle |
| pixel_capacity | parameter | src/framebuffer/framebuffer.c | pstvnc_framebuffer_init | local | Gives available pixel storage capacity independent of logical geometry. | desktop framebuffer lifecycle |
| pstvnc_framebuffer_set_geometry | function | src/framebuffer/framebuffer.c | desktop framebuffer | public | Sets logical geometry only when storage is sufficient and revokes prior authority. | authoritative framebuffer semantics |
| framebuffer | parameter | src/framebuffer/framebuffer.c | pstvnc_framebuffer_set_geometry | local | Supplies the framebuffer whose logical geometry is changed. | desktop framebuffer lifecycle |
| width | parameter | src/framebuffer/framebuffer.c | pstvnc_framebuffer_set_geometry | local | Gives the requested nonzero logical width. | desktop framebuffer lifecycle |
| height | parameter | src/framebuffer/framebuffer.c | pstvnc_framebuffer_set_geometry | local | Gives the requested nonzero logical height. | desktop framebuffer lifecycle |
| required | variable | src/framebuffer/framebuffer.c | pstvnc_framebuffer_set_geometry | local | Stores required pixel capacity for the requested geometry. | desktop framebuffer lifecycle |
| pstvnc_framebuffer_pixel_count | function | src/framebuffer/framebuffer.c | desktop framebuffer | public | Returns the logical pixel count represented by current framebuffer geometry. | authoritative framebuffer semantics |
| framebuffer | parameter | src/framebuffer/framebuffer.c | pstvnc_framebuffer_pixel_count | local | Supplies geometry used to compute the logical pixel count. | desktop framebuffer lifecycle |
| pstvnc_framebuffer_invalidate | function | src/framebuffer/framebuffer.c | desktop framebuffer | public | Revokes whole-frame authority and clears presentation dirty state. | authoritative framebuffer semantics |
| framebuffer | parameter | src/framebuffer/framebuffer.c | pstvnc_framebuffer_invalidate | local | Supplies the framebuffer whose authority is revoked. | failure semantics |
| pstvnc_framebuffer_mark_valid | function | src/framebuffer/framebuffer.c | desktop framebuffer | public | Records an external complete-frame proof by publishing framebuffer authority. | authoritative framebuffer semantics |
| framebuffer | parameter | src/framebuffer/framebuffer.c | pstvnc_framebuffer_mark_valid | local | Supplies the initialized framebuffer to promote after session proof. | authoritative framebuffer semantics |
| pstvnc_framebuffer_write_rect | function | src/framebuffer/framebuffer.c | desktop framebuffer | public | Copies a bounded source rectangle into CPU authority storage and records dirty bounds. | authoritative framebuffer semantics |
| framebuffer | parameter | src/framebuffer/framebuffer.c | pstvnc_framebuffer_write_rect | local | Supplies destination framebuffer storage and geometry. | framebuffer rectangle writes |
| x | parameter | src/framebuffer/framebuffer.c | pstvnc_framebuffer_write_rect | local | Gives destination rectangle left coordinate. | framebuffer rectangle writes |
| y | parameter | src/framebuffer/framebuffer.c | pstvnc_framebuffer_write_rect | local | Gives destination rectangle top coordinate. | framebuffer rectangle writes |
| width | parameter | src/framebuffer/framebuffer.c | pstvnc_framebuffer_write_rect | local | Gives destination rectangle width. | framebuffer rectangle writes |
| height | parameter | src/framebuffer/framebuffer.c | pstvnc_framebuffer_write_rect | local | Gives destination rectangle height. | framebuffer rectangle writes |
| source | parameter | src/framebuffer/framebuffer.c | pstvnc_framebuffer_write_rect | local | Points to source pixels that will be copied into framebuffer storage. | framebuffer rectangle writes |
| source_stride | parameter | src/framebuffer/framebuffer.c | pstvnc_framebuffer_write_rect | local | Gives source row stride and must cover the requested width. | framebuffer rectangle writes |
| row | variable | src/framebuffer/framebuffer.c | pstvnc_framebuffer_write_rect | local | Iterates over source and destination rows during rectangle copy. | framebuffer rectangle writes |
| destination | variable | src/framebuffer/framebuffer.c | pstvnc_framebuffer_write_rect | local | Points at the current destination row within authoritative storage. | framebuffer rectangle writes |
| source_row | variable | src/framebuffer/framebuffer.c | pstvnc_framebuffer_write_rect | local | Points at the current source row selected by source stride. | framebuffer rectangle writes |
| pstvnc_framebuffer_clear_dirty | function | src/framebuffer/framebuffer.c | desktop framebuffer | public | Clears dirty state without changing whole-frame validity or pixel contents. | framebuffer dirty state |
| framebuffer | parameter | src/framebuffer/framebuffer.c | pstvnc_framebuffer_clear_dirty | local | Supplies mutable framebuffer dirty-state ownership. | framebuffer dirty state |
| pstvnc_framebuffer_get_dirty | function | src/framebuffer/framebuffer.c | desktop framebuffer | public | Copies the current conservative dirty rectangle when one exists. | framebuffer dirty state |
| framebuffer | parameter | src/framebuffer/framebuffer.c | pstvnc_framebuffer_get_dirty | local | Supplies the framebuffer whose dirty state is inspected. | framebuffer dirty state |
| rect | parameter | src/framebuffer/framebuffer.c | pstvnc_framebuffer_get_dirty | local | Receives a copy of the current dirty bounding rectangle. | framebuffer dirty state |
| PSTVNC_FRAMEBUFFER_H | include guard | src/framebuffer/framebuffer.h | desktop framebuffer interface | file | Prevents repeated inclusion of framebuffer types and declarations. | clean source interface |
| pstvnc_framebuffer_rect | structure | src/framebuffer/framebuffer.h | desktop framebuffer interface | public | Defines an axis-aligned logical rectangle used for dirty-state reporting. | desktop framebuffer |
| x | field | src/framebuffer/framebuffer.h | pstvnc_framebuffer_rect | public | Stores the rectangle left coordinate in logical pixels. | desktop framebuffer |
| y | field | src/framebuffer/framebuffer.h | pstvnc_framebuffer_rect | public | Stores the rectangle top coordinate in logical pixels. | desktop framebuffer |
| width | field | src/framebuffer/framebuffer.h | pstvnc_framebuffer_rect | public | Stores the rectangle width in logical pixels. | desktop framebuffer |
| height | field | src/framebuffer/framebuffer.h | pstvnc_framebuffer_rect | public | Stores the rectangle height in logical pixels. | desktop framebuffer |
| pstvnc_framebuffer_rect_t | type | src/framebuffer/framebuffer.h | desktop framebuffer interface | public | Names the dirty-rectangle value type. | desktop framebuffer |
| pstvnc_framebuffer | structure | src/framebuffer/framebuffer.h | desktop framebuffer interface | public | Defines storage, geometry, authority, and dirty state for the remote image. | CLEAN_ARCHITECTURE: Desktop framebuffer |
| pixels | field | src/framebuffer/framebuffer.h | pstvnc_framebuffer | public | Points to caller-owned CPU pixel storage for the remote desktop. | desktop framebuffer |
| pixel_capacity | field | src/framebuffer/framebuffer.h | pstvnc_framebuffer | public | Records allocated pixel capacity independently of current geometry. | desktop framebuffer |
| width | field | src/framebuffer/framebuffer.h | pstvnc_framebuffer | public | Records current logical framebuffer width. | desktop framebuffer |
| height | field | src/framebuffer/framebuffer.h | pstvnc_framebuffer | public | Records current logical framebuffer height. | desktop framebuffer |
| valid | field | src/framebuffer/framebuffer.h | pstvnc_framebuffer | public | States whether every logical pixel is currently authoritative desktop data. | authoritative framebuffer semantics |
| dirty | field | src/framebuffer/framebuffer.h | pstvnc_framebuffer | public | States whether the latest accepted server update changed any pixel. | framebuffer dirty state |
| dirty_rect | field | src/framebuffer/framebuffer.h | pstvnc_framebuffer | public | Stores a conservative bounding rectangle for the latest pixel changes. | framebuffer dirty state |
| pstvnc_framebuffer_t | type | src/framebuffer/framebuffer.h | desktop framebuffer interface | public | Names the authoritative framebuffer metadata and storage view. | CLEAN_ARCHITECTURE: Desktop framebuffer |
| pstvnc_framebuffer_init | function declaration | src/framebuffer/framebuffer.h | desktop framebuffer interface | public | Declares framebuffer initialization around caller-owned storage. | desktop framebuffer lifecycle |
| pstvnc_framebuffer_set_geometry | function declaration | src/framebuffer/framebuffer.h | desktop framebuffer interface | public | Declares validated geometry changes that revoke old pixel authority. | desktop framebuffer lifecycle |
| pstvnc_framebuffer_pixel_count | function declaration | src/framebuffer/framebuffer.h | desktop framebuffer interface | public | Declares logical pixel-count computation. | desktop framebuffer lifecycle |
| pstvnc_framebuffer_invalidate | function declaration | src/framebuffer/framebuffer.h | desktop framebuffer interface | public | Declares authority revocation and dirty-state clearing. | authoritative framebuffer semantics |
| pstvnc_framebuffer_mark_valid | function declaration | src/framebuffer/framebuffer.h | desktop framebuffer interface | public | Declares publication of an externally proven complete framebuffer. | authoritative framebuffer semantics |
| pstvnc_framebuffer_write_rect | function declaration | src/framebuffer/framebuffer.h | desktop framebuffer interface | public | Declares bounded rectangle copying into CPU framebuffer storage. | framebuffer rectangle writes |
| pstvnc_framebuffer_clear_dirty | function declaration | src/framebuffer/framebuffer.h | desktop framebuffer interface | public | Declares reset of dirty presentation state. | framebuffer dirty state |
| pstvnc_framebuffer_get_dirty | function declaration | src/framebuffer/framebuffer.h | desktop framebuffer interface | public | Declares inspection of the current dirty bounding rectangle. | framebuffer dirty state |
| framebuffer | prototype parameter | src/framebuffer/framebuffer.h | pstvnc_framebuffer_clear_dirty | prototype | Supplies mutable framebuffer dirty-state ownership. | framebuffer dirty state |
| framebuffer | prototype parameter | src/framebuffer/framebuffer.h | pstvnc_framebuffer_get_dirty | prototype | Supplies the framebuffer whose dirty state is inspected. | framebuffer dirty state |
| rect | prototype parameter | src/framebuffer/framebuffer.h | pstvnc_framebuffer_get_dirty | prototype | Receives a copy of the current dirty bounding rectangle. | framebuffer dirty state |
| framebuffer | prototype parameter | src/framebuffer/framebuffer.h | pstvnc_framebuffer_init | prototype | Supplies the framebuffer value whose metadata is initialized. | desktop framebuffer lifecycle |
| pixel_capacity | prototype parameter | src/framebuffer/framebuffer.h | pstvnc_framebuffer_init | prototype | Gives available pixel storage capacity independent of logical geometry. | desktop framebuffer lifecycle |
| pixels | prototype parameter | src/framebuffer/framebuffer.h | pstvnc_framebuffer_init | prototype | Supplies caller-owned CPU pixel storage without asserting content authority. | desktop framebuffer lifecycle |
| framebuffer | prototype parameter | src/framebuffer/framebuffer.h | pstvnc_framebuffer_invalidate | prototype | Supplies the framebuffer whose authority is revoked. | failure semantics |
| framebuffer | prototype parameter | src/framebuffer/framebuffer.h | pstvnc_framebuffer_mark_valid | prototype | Supplies the initialized framebuffer to promote after session proof. | authoritative framebuffer semantics |
| framebuffer | prototype parameter | src/framebuffer/framebuffer.h | pstvnc_framebuffer_pixel_count | prototype | Supplies geometry used to compute the logical pixel count. | desktop framebuffer lifecycle |
| framebuffer | prototype parameter | src/framebuffer/framebuffer.h | pstvnc_framebuffer_set_geometry | prototype | Supplies the framebuffer whose logical geometry is changed. | desktop framebuffer lifecycle |
| height | prototype parameter | src/framebuffer/framebuffer.h | pstvnc_framebuffer_set_geometry | prototype | Gives the requested nonzero logical height. | desktop framebuffer lifecycle |
| width | prototype parameter | src/framebuffer/framebuffer.h | pstvnc_framebuffer_set_geometry | prototype | Gives the requested nonzero logical width. | desktop framebuffer lifecycle |
| framebuffer | prototype parameter | src/framebuffer/framebuffer.h | pstvnc_framebuffer_write_rect | prototype | Supplies destination framebuffer storage and geometry. | framebuffer rectangle writes |
| height | prototype parameter | src/framebuffer/framebuffer.h | pstvnc_framebuffer_write_rect | prototype | Gives destination rectangle height. | framebuffer rectangle writes |
| source | prototype parameter | src/framebuffer/framebuffer.h | pstvnc_framebuffer_write_rect | prototype | Points to source pixels that will be copied into framebuffer storage. | framebuffer rectangle writes |
| source_stride | prototype parameter | src/framebuffer/framebuffer.h | pstvnc_framebuffer_write_rect | prototype | Gives source row stride and must cover the requested width. | framebuffer rectangle writes |
| width | prototype parameter | src/framebuffer/framebuffer.h | pstvnc_framebuffer_write_rect | prototype | Gives destination rectangle width. | framebuffer rectangle writes |
| x | prototype parameter | src/framebuffer/framebuffer.h | pstvnc_framebuffer_write_rect | prototype | Gives destination rectangle left coordinate. | framebuffer rectangle writes |
| y | prototype parameter | src/framebuffer/framebuffer.h | pstvnc_framebuffer_write_rect | prototype | Gives destination rectangle top coordinate. | framebuffer rectangle writes |
