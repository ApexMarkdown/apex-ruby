# Apex Ruby Bindings

Ruby bindings for the [Apex](https://github.com/ApexMarkdown/apex) unified Markdown processor.

Apex is a C library that supports CommonMark, GFM, MultiMarkdown, Kramdown, and a number of
Marked-specific extensions. This gem vendors the Apex sources and exposes a small, kramdown-style
Ruby API for converting Markdown to HTML.

## Requirements

Before installing the gem, you must have the **cmark-gfm** C library available on your system. The gem compiles a native extension that links against it.

- **macOS (Homebrew):**  
  ```sh
  brew install cmark-gfm
  ```
- **Other platforms:** Install the `cmark-gfm` package for your distribution and ensure development headers and `pkg-config` are available. See [cmark-gfm](https://github.com/github/cmark-gfm) for build instructions.

If you run `gem install apex-ruby` without cmark-gfm installed, the build will fail with a clear error message and the same installation instructions.

## Installation

Add this line to your application's Gemfile:

```ruby
gem "apex-ruby"
```

And then execute:

```sh
bundle install
```

Or install it yourself as:

```sh
gem install apex-ruby
```

## Usage

The main API mirrors the simplicity of the `kramdown` gem:

```ruby
require "apex"

html = Apex::Document.markdown_to_html(text)
```

### Modes

You can select a compatibility mode via the `mode:` keyword argument:

```ruby
html = Apex::Document.markdown_to_html(text, mode: :unified)      # default
html = Apex::Document.markdown_to_html(text, mode: :gfm)          # or :github
html = Apex::Document.markdown_to_html(text, mode: :multimarkdown) # or :mmd
html = Apex::Document.markdown_to_html(text, mode: :commonmark)   # or :cmark
html = Apex::Document.markdown_to_html(text, mode: :kramdown)
```

### Options

Any additional keyword arguments are mapped directly to the underlying `apex_options` struct
defined in `apex.h`. For example:

```ruby
html = Apex::Document.markdown_to_html(
  text,
  mode: :unified,
  enable_tables: true,
  enable_footnotes: true,
  generate_header_ids: true,
  relaxed_tables: true,
  wikilink_extension: "html"
)
```

See the Apex documentation for the complete list of options and their semantics.

### Instance API

If you prefer an object-oriented style (similar to `Kramdown::Document`), you can use
`Apex::Document` instances:

```ruby
doc = Apex::Document.new(text, mode: :gfm, enable_tables: true)
html = doc.to_html
```

### Jekyll

You can use Apex as Jekyll’s Markdown converter instead of Kramdown via a custom converter plugin. See **[Jekyll-Apex.md](Jekyll-Apex.md)** for step-by-step instructions. *That integration is currently untested and documented as a placeholder; feedback from anyone willing to try it is welcome at [ApexMarkdown/apex-ruby/issues](https://github.com/ApexMarkdown/apex-ruby/issues).*

## Development

To run the test suite:

```sh
bundle exec rake test
```

To build and install the gem into all Ruby versions managed by [mise](https://mise.jdx.dev/):

```sh
bundle exec rake install
```

This will:

1. Build `pkg/apex-ruby-<version>.gem`
2. Run `gem install` for each Ruby reported by `mise ls ruby`.

## License

This project is licensed under the MIT license. See the Apex repository for the license of the
underlying C implementation.

