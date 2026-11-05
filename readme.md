# OrbitC User Manual

## Overview

OrbitC is a modular C99 library designed for embedded and flight software environments.

The library provides reusable low\-level services with a focus on portability, deterministic behavior, explicit error handling, controlled memory usage, and compile\-time configurability.

OrbitC is designed to remain independent from the standard C library and can be built in freestanding environments.

## Architecture

OrbitC is organized as a collection of independent modules.

Typical modules provide services such as:

- primitive types and platform abstractions;
- status and diagnostic handling;
- memory manipulation;
- safe string operations;
- checksums;
- memory allocation.

Additional modules can be added without changing the general integration model.

Each module is located under:

modules/<module>/

and provides its own:

modules/<module>/um.md

The module User Manual describes its dependencies, configuration options, public API, safety considerations, and typical usage.

Refer to the corresponding um.md before integrating a module.

## Global Configuration

The global library configuration is defined in:

config.mk

It controls the compiler, target architecture, optimization backends, assertions, enabled modules, and build directories.

### Target Configuration

The target architecture can be configured using:

CPU = generic

CPU\_FLAGS =

ORBITC\_ARCH\_BITS = 64

ORBITC\_LITTLE\_ENDIAN = 1

ORBITC\_ALLOW\_UNALIGNED = 0

These values must match the target processor and ABI.

### Module Selection

Modules can be enabled or removed from the build independently:

ENABLE\_TYPES = 1

ENABLE\_STATUS = 1

ENABLE\_MEMORY = 1

ENABLE\_STRINGS = 1

ENABLE\_CHECKSUMS = 1

ENABLE\_ALLOCATOR = 1

Setting a module to 0 removes it from the build.

Module dependencies must remain enabled. Refer to each module's um.md for dependency requirements.

### Backend Selection

Some modules provide alternative implementations optimized for different constraints.

For example:

ORBITC\_MEM\_BACKEND = 1

selects the memory implementation backend.

ORBITC\_CRC\_BACKEND = 1

selects the checksum implementation backend.

Backend availability and behavior are documented in the corresponding module User Manual.

## Module\-Specific Configuration

Modules requiring additional compile\-time options provide their own configuration file:

modules/<module>/<module>\-config.mk

These configuration files are included by the main build system when the corresponding module is enabled.

This allows each module to expose specialized configuration without adding every option to the global config.mk.

## Building OrbitC

Build the library with:

make

The resulting static library is generated as:

build/liborbitc.a

Build artifacts are stored under:

build/

The complete build directory can be removed with:

make clean

## Running Unit Tests

OrbitC uses Unity for unit testing.

The Unity installation can be configured using:

UNITY\_ROOT = /path/to/unity

Build the test executable with:

make test

Build and execute the complete enabled test suite with:

make run\_tests

Only tests associated with enabled modules are included.

## Integration

Applications should include the headers of the OrbitC modules they require and link against:

liborbitc.a

The same architecture, backend, and module configuration used to build OrbitC must be preserved when compiling code that depends on configuration\-sensitive OrbitC interfaces.

A typical embedded configuration can be provided directly through config.mk or overridden from the build environment.

For example:

make ORBITC\_ARCH\_BITS=32 ORBITC\_MEM\_BACKEND=0

Target\-specific compiler options can be provided using:

CPU\_FLAGS

This can be used for options such as processor selection, ABI configuration, or hardware\-specific optimization flags.

## Freestanding Builds

The production configuration uses C99 in freestanding mode:

\-std=c99 \-ffreestanding \-nostdinc \-fno\-builtin

OrbitC therefore does not assume the availability of a hosted C standard library.

This makes the library suitable as a low\-level foundation for bare\-metal systems, RTOS\-based applications, flight software, and other constrained embedded environments.

## Adding New Modules

New functionality should be implemented as an independent module under:

modules/<module>/

A module should provide its own public interface, implementation, build integration, tests, and configuration when required.

It should also provide:

modules/<module>/um.md

The User Manual is the reference for integrating and using the module.

The global OrbitC documentation should remain generic. Module\-specific APIs and configuration details belong in their respective User Manuals.

