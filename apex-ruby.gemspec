require_relative "lib/apex/version"

Gem::Specification.new do |spec|
  spec.name          = 'apex-ruby'
  spec.version       = Apex::VERSION
  spec.authors       = ['Brett Terpstra']
  spec.email         = ['me@brettterpstra.com']

  spec.summary       = 'Ruby bindings for the Apex unified Markdown processor'
  spec.description   = 'Apex is a unified Markdown engine with CommonMark, GFM, MultiMarkdown, and Kramdown compatibility. This gem provides a kramdown-style Ruby API backed by the Apex C library.'
  spec.homepage      = 'https://github.com/ApexMarkdown/apex-ruby'
  spec.license       = 'MIT'

  spec.required_ruby_version = '>= 3.0'

  # Only the tracked Apex and cmark-gfm sources the extension build needs;
  # the rest of the apex checkout (tests, docs, build dirs, logs) stays out.
  apex_src_dir = File.join(__dir__, 'ext', 'apex_ext', 'apex_src')
  apex_src_pattern = %r{\A(?:LICENSE|include/.+\.(?:h|modulemap)|src/.+\.[ch]|PackageSupport/cmark-gfm/[^/]+\.h|
                          vendor/cmark-gfm/(?:COPYING|src/[^/]+\.(?:c|h|inc)|extensions/[^/]+\.[ch]))\z}x
  apex_src_files = Dir.chdir(apex_src_dir) do
    tracked = `git ls-files -z --recurse-submodules 2>/dev/null`.split("\0")
    tracked = Dir.glob('**/*') if tracked.empty?
    tracked.grep(apex_src_pattern)
  end

  spec.files = Dir.chdir(__dir__) do
    Dir[
      'README.md',
      'apex-ruby.gemspec',
      'lib/**/*.rb',
      'ext/apex_ext/apex_ext.c',
      'ext/apex_ext/extconf.rb'
    ]
  end + apex_src_files.map { |f| "ext/apex_ext/apex_src/#{f}" }

  spec.require_paths = ['lib']
  spec.extensions    = ['ext/apex_ext/extconf.rb']

  spec.metadata['source_code_uri'] = spec.homepage
  spec.metadata['changelog_uri']   = "#{spec.homepage}/blob/main/CHANGELOG.md"
end
