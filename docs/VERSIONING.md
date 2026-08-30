# PS-to-VNC Versioning

## Product version

PS-to-VNC uses Semantic Versioning-style product identifiers:

    MAJOR.MINOR.PATCH

Pre-release identifiers are used while the project is still maturing:

    0.1.0-alpha.1
    0.1.0-beta.1
    1.0.0-rc.1
    1.0.0

The authoritative plain product version is stored in the repository root:

    VERSION

Git release tags use a leading `v`:

    v0.1.0-alpha.1

## Version and engineering milestone are different identities

Engineering identifiers such as:

    M3Z
    M4A
    M4B

identify migrations, experiments, qualification events, or evidence sets.

They are retained because they are useful forensic engineering history.

They are not product versions.

A single Git commit can therefore be both:

- the authority for an engineering milestone;
- and the source represented by a product release tag.

## Pre-1.0 policy

Before `1.0.0`, interfaces and architecture are explicitly still evolving.

PATCH increments are normally used for narrow fixes:

    0.2.0 -> 0.2.1

MINOR increments are normally used for meaningful capability or
architectural milestones:

    0.2.1 -> 0.3.0

Because the major version is zero, incompatible internal or user-facing
changes may occur between minor versions. Such changes must still be
documented.

Pre-release suffixes distinguish development maturity:

    alpha
        architecture and features may still change substantially

    beta
        architecture and intended feature set are substantially established

    rc
        release candidate believed capable of becoming the associated
        release without further functional changes

## 1.0.0

Version `1.0.0` is not assigned merely because development has existed for
a long time.

The 1.0 release should represent a deliberately satisfied project
readiness checklist covering architecture, core behavior, reproducible
installation, recovery, configuration, testing, documentation, and
deployment constraints.

## Single version authority

The `VERSION` file is the only manually maintained product-version value.

Future build-time version headers or UI strings must be generated from
`VERSION`.

The project must not create several manually synchronized version
definitions.

## Binary identity and release identity

These identities serve different purposes:

    product version
        human-facing release identity

    Git commit
        exact repository/source identity

    ELF SHA256
        exact executable identity

    load-image SHA256
        exact PS2 load-image identity

    hardware authority
        evidence describing what was physically qualified

A version number must never be used as a substitute for an exact binary
hash in qualification evidence.

## Initial baseline

The first versioned development baseline is:

    0.1.0-alpha.1

It represents:

- completion of coarse structural migration;
- twelve real translation units;
- zero remaining coarse-migration blockers;
- M4A machine and physical hardware qualification;
- and the beginning of architectural normalization.

The already-qualified ELF does not contain the product version string.
Changing it solely to embed the version would create a new binary after
hardware qualification.

When executable version reporting is introduced, the build must generate
it from `VERSION`; no duplicate hand-maintained string will be added.
