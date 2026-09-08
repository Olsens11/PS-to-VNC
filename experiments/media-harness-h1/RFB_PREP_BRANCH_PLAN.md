# RFB mux source-preparation branch plan

Source-changing RFB mux preparation should be isolated from the currently
qualified H1 experiment branch. Create a dedicated descendant branch from the
latest documentation/preparation checkpoint before changing C, headers, build
rules, or Pi bridge behavior.

Preferred branch name:

    experiment/h1-rfb-mux-prep

The parent branch remains `experiment/media-harness-h1`.
