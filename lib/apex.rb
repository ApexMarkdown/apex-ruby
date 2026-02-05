# frozen_string_literal: true

require "apex/version"

# In development (running from the source tree), ensure the compiled
# native extension under ./ext is on the load path so that
# `require "apex_ext/apex_ext"` can find it. In an installed gem this
# is handled by RubyGems.
ext_dir = File.expand_path("../ext", __dir__)
$LOAD_PATH.unshift(ext_dir) unless $LOAD_PATH.include?(ext_dir)

require "apex_ext/apex_ext"

require_relative "apex/configurable"
require_relative "apex/document"

# Apex Markdown Ruby bindings.
#
# The primary entry point is `Apex::Document`, which provides a
# kramdown-style API:
#   Apex::Document.markdown_to_html(text, mode: :unified, **options)
#
# Options are mapped onto the underlying `apex_options` struct from the
# C library. See the Apex documentation for the full list of supported
# options and defaults.
module Apex
end

