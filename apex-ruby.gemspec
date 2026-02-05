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

  spec.files = Dir.chdir(__dir__) do
    Dir[
      'README.md',
      'apex-ruby.gemspec',
      'lib/**/*.rb',
      'ext/**/*.{c,h,rb}',
      'ext/apex_ext/apex_src/**/*'
    ]
  end

  spec.require_paths = ['lib']
  spec.extensions    = ['ext/apex_ext/extconf.rb']

  spec.metadata['source_code_uri'] = spec.homepage
  spec.metadata['changelog_uri']   = "#{spec.homepage}/blob/main/CHANGELOG.md"
end
