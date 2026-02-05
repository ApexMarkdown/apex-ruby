# Using Apex with Jekyll

**Note:** This integration is **untested**. The documentation below serves as a placeholder until it has been tried on real Jekyll sites. If you try it, I'd love your feedback -- please open an issue or comment at [ApexMarkdown/apex-ruby/issues](https://github.com/ApexMarkdown/apex-ruby/issues).

---

You can use the [apex](https://github.com/ApexMarkdown/apex-ruby) gem as Jekyll's Markdown converter instead of Kramdown, without monkey-patching. Jekyll's converter system lets you register a custom converter and select it in `_config.yml`.

## 1. Add the gem

In your Jekyll project's `Gemfile`:

```ruby
gem "apex-ruby"
```

Then run:

```sh
bundle install
```

Ensure [cmark-gfm is installed](https://github.com/ApexMarkdown/apex-ruby#requirements) on your system so the gem can build.

## 2. Add a converter plugin

Create a file in your Jekyll `_plugins` directory (e.g. `_plugins/apex_converter.rb`):

```ruby
# _plugins/apex_converter.rb
require "apex"

module Jekyll
  class ApexConverter < Converter
    safe true
    priority :low

    def matches(ext)
      ext =~ %r!^\.(markdown|md|mkd|mkdn)$!i
    end

    def output_ext(ext)
      ".html"
    end

    def convert(content)
      # use mode: kramdown for Kramdown compatibility (limits functionality)
      Apex::Document.markdown_to_html(content, mode: :unified)
    end
  end
end
```

Jekyll loads any `.rb` files in `_plugins/`. This converter runs for `.markdown` and `.md` (and the other matched extensions) and uses Apex with unified mode (covers Kramdown, CommonMark, and more syntax). Use `mode: :kramdown` for behavior closer to Kramdown.

## 3. Use it in `_config.yml`

Tell Jekyll to use this converter for Markdown:

```yaml
markdown: ApexConverter
```

Use the same name as your converter class. You do **not** need to reference or patch Kramdown; Jekyll will use your converter instead of the default.

## 4. Optional: pass options from config

To drive Apex options from `_config.yml`, read a config key in the converter:

```ruby
def convert(content)
  opts = @config["apex"] || {}
  mode = (opts["mode"] || "kramdown").to_sym
  Apex::Document.markdown_to_html(content, mode: mode, **opts.reject { |k, _| k == "mode" })
end
```

Then in `_config.yml`:

```yaml
markdown: ApexConverter
apex:
  mode: kramdown
  enable_tables: true
  generate_header_ids: true
```

---

**Feedback:** This flow has not been validated on a full Jekyll site. If you try it, please share your experience or report issues at [ApexMarkdown/apex-ruby/issues](https://github.com/ApexMarkdown/apex-ruby/issues).
