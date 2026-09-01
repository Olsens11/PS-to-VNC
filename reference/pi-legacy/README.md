# Legacy Raspberry Pi Reference

## Status

    ROLE=HISTORICAL_REFERENCE_ONLY
    SOURCE_CAPTURE=2026-08-31
    CLEAN_PI_AUTHORITY=NO
    GITHUB_ISSUE=5
    SANITIZED=YES

This tree preserves the product-significant parts of the exploratory PS2VNC Raspberry Pi environment so the clean companion can be compared against proven historical behavior without cloning the old machine.

It is **not** an installer and is **not** clean Pi configuration authority. Every package, service, and setting must still be classified through the clean dependency ledger before adoption.

## Provenance

The source was the verified preservation set copied from the old Pi before the clean 1 TB development card was brought up.

Uploaded reference bundle:

    UPLOAD_SHA256=8ff63fca4771998668e237cec7e9c0818673f22385ff14c2c2abdc11c5cdd8da

Embedded exact configuration snapshot:

    CONFIG_SNAPSHOT_SHA256=89c1fb8ccc627083e3175c14682ba34380539ff98b63d2d36764dbebcd9349c8

The original preservation backup remains the byte-authoritative source. Files in this Git tree are a curated convenience/reference view.

## Included

- old Pi OS/hardware identity;
- relevant/manual package census;
- enabled-service census;
- sanitized PS2-facing Ethernet/VNC runtime summary;
- final PS2VNC systemd service graph;
- sanitized PS2 Ethernet NetworkManager profile;
- effective Samba configuration.

## Deliberately excluded

The following remain in the verified preservation archive but are not committed:

- Wi-Fi NetworkManager profiles, because the preserved files contain plaintext credentials;
- household Wi-Fi SSIDs, addresses, IPv6 addresses, and interface MAC addresses;
- filesystem/partition UUIDs and unrelated storage inventory;
- Docker/cache/storage-size material unrelated to the companion runtime;
- historical backup copies of the management server;
- the old user-local `deploy-ps2vnc` helper, which was development-local and not part of the clean runtime contract;
- PS2 toolchain binaries and other large development-machine artifacts;
- game/media contents;
- secrets, SSH keys, tokens, and credentials.

The preserved final runtime scripts and desktop configuration remain available in the verified backup and can be imported individually if a clean-Pi adoption decision requires exact source comparison. Their behavior is already summarized by the B13 semantic audit.

## Interpretation rule

Historical presence is not adoption.

For each clean-Pi dependency or setting, compare this tree with the virgin supported Raspberry Pi OS baseline and classify it as one of:

- `OS_BASE`
- `EVALUATING`
- `ADOPTED_RUNTIME`
- `ADOPTED_BUILD`
- `ADOPTED_OPTIONAL`
- `REJECTED`
- `REPLACED`

When a clean implementation differs from this reference, record why.
