# PDB Decompiler

* [About](#about)
* [Usage](#usage)
* [Contributing](#contributing)

## About

A tool to decompile MSVC PDB files to C++ source code.

This tool is a work in progress and will most likely crash, have duplicate output, have invalid output, or miss output entirely. Feel free to file an issue or submit a pull request.

## Usage

```
cargo run --release -- --out=/path/to/out/ --pdb=/path/to/file.pdb --base-address=0x180000000 > file.pdb.log
```

### Flags

| Short | Long | Description |
|-|-|-|
| `-c` | `--export-cpp` | Whether to export C++ code. |
| | `--export-function-types` | Whether to generate IDA script statements that try to set function types. |
| | `--export-pseudocode-to-files` | Whether to generate IDA script statements that export pseudocode to their appropriate source files. |
| | `--export-pseudocode-to-json` | Whether to generate IDA script statements that export pseudocode to a JSON mapping file. |
| | `--force-no-inlines` | Forces a `__declspec(noinline)` attribute for functions that were never inlined but did not have the attribute. |
| `-h` | `--help` | Prints help information. |
| `-r` | `--reorganize` | Whether to reorganize generated C++ code to Bungie's coding standards. (Experimental) |
| `-u` | `--unroll-functions` | Whether to include scope information in decompiled function stubs. (Experimental) |
| | `--verbose-blocks` | Whether to write extra block level information. |
| `-V` | `--version` | Prints version information. |

### Options

| Short | Long | Description |
|-|-|-|
| `-b` | `--base-address <base-address>` | The base address to add when resolving an RVA. (Optional) |
| | `--export-debug-blocks-json <export-debug-blocks-json>` | Dumps each procedure's unrolled scope tree (variable declarations in nested blocks) to a JSON file keyed by mangled function name. (Forces unrolling.) |
| | `--function-scopes-out <function-scopes-out>` | The output directory to dump all function scopes C++ code to. |
| | `--function-scopes-pdb <function-scopes-pdb>` | The file path to the MSVC PDB file to decompile for extra function scope information. |
| `-o` | `--out <out>` | The output directory to dump all C++ code to. |
| `-p` | `--pdb <pdb>` | The file path to the MSVC PDB file to decompile. |
| | `--pseudocode-json-path <pseudocode-json-path>` | The file containing all function pseudocode in a JSON mapping. (Optional) |
| | `--project-name` | The file and folder name for used to generate the umbrella include header. The emitted include is `{project_name}/{project_name}.h` (e.g. `project/project.h`). |
| | `--include-compiler-generated` | Includes compiler-generated procedure definitions (implicit ctors/dtors/assignment, thunks, dynamic initializers) in output C++ code |
| | `--source-path <source-path>` | A source file path (or directory) to filter which modules are decompiled. Only modules whose source path is equal to, or nested under, one of these paths are exported. Can be specified multiple times. For a directory path, output is rebased so the last folder becomes the output root (e.g. `--source-path .../FolderA/SubFolderA` writes `.../FolderA/SubFolderA/foo.cpp` to `<out>/SubFolderA/foo.cpp`). |

## Contributing

Any assistance or valid criticism would be appreciated. Please feel free to have a look at some of the open [issues](https://github.com/camden-smallwood/pdb-decompiler/issues), especially those tagged with [help wanted](https://github.com/camden-smallwood/pdb-decompiler/issues?q=is%3Aissue+is%3Aopen+label%3A"help+wanted").

## Legacy Xbox 360 PDBs (XDK 9328)

Pre-IPI PDBs are detected from the PDB information stream's feature signatures.
Stream 4 can contain `/names` in these files; it must not be parsed as IPI.
The legacy path recovers:

- Class, union, enum, and function types from TPI. With `--export-cpp`, types
  without declaration-file metadata and global typedefs are written to `exported.h`.
- Module source paths and build metadata from `S_COMPILE2` command blocks or
  `S_ENVBLOCK`, with DBI source-file lists and line records as fallbacks.
- Aligned C13 source-line records from XDK MASM modules.
- Procedure names, addresses, parameters, locals, module typedefs, and data from
  module/global symbols using the existing C++ and IDA exporters.
- Original build environment strings, including Xbox compiler switches, as
  `Build ...` comments. These switches are preserved rather than interpreted
  using the desktop compiler's argument grammar.

Modules with useful symbols but no identifiable source file are preserved under
`__pdb_modules/module_<stream>.cpp`. Source filters still apply to these paths.
`exported.h` is a shared type inventory and is not limited by source filters.
Legacy procedures without source lines are retained: lack of line information
alone cannot distinguish handwritten assembly or stripped objects from
compiler-generated functions.

A PDB cannot supply information the compiler never emitted. In particular,
legacy records do not necessarily identify a type's original header/line or
individual inline call sites. Forward-only types remain declarations. Function
bodies remain debug-scope stubs unless pseudocode is supplied through the IDA
workflow; this tool does not reconstruct machine-code behavior by itself.

Run unit tests with `cargo test`. A generated fixture test exercises the installed
XDK compiler and linker without redistributing SDK files:

```powershell
$env:XDK_9328_ROOT = 'D:\Documents\Reverse Engineering\SDKS\XDK\9328'
cargo test --test xdk9328 -- --ignored
```

The test deliberately gives the object a different filename from its source and
checks recovered types, build metadata, symbols, scope variables, and repeatable
shared-header output. It also removes `/names` and the build source key from a
generated copy to verify the DBI-only source lookup.
