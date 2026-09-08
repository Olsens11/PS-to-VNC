# Build wrapper for the generated cumulative through-Issue-39 interactive H1.
#
# The generated transport header intentionally keeps the canonical include guard.
# Force it before every ordinary EE C translation unit so quoted includes of the
# canonical h1_transport_runtime.h are suppressed and every object sees the same
# extended transport-runtime ABI. Canonical source files remain unchanged.

include mk/media-harness-h1-cumulative39-interactive.mk

H1_INTERACTIVE_TRANSPORT_HEADER := \
	experiments/media-harness-h1/cumulative39-interactive-include/h1_transport_runtime.h

EE_CFLAGS := -include $(H1_INTERACTIVE_TRANSPORT_HEADER) $(EE_CFLAGS)
