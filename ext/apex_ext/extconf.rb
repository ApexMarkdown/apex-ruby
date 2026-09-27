require "mkmf"

apex_src      = File.expand_path('apex_src', __dir__)
apex_incl     = File.join(apex_src, 'include')
apex_srcdir   = File.join(apex_src, 'src')
apex_extdir   = File.join(apex_srcdir, 'extensions')
vendor_cmark  = File.join(apex_src, 'vendor', 'cmark-gfm')
config_incl_dir = File.join(apex_src, 'PackageSupport', 'cmark-gfm')
$INCFLAGS << " -I#{apex_incl}"
$INCFLAGS << " -I#{config_incl_dir}"

# ---- cmark-gfm (bundled) ---------------------------------------------------
#
# Apex depends on its patched cmark-gfm (apex_src/vendor/cmark-gfm), and its
# extensions use cmark-gfm's internal structs. Linking a system libcmark-gfm
# (e.g. /usr/lib/libcmark-gfm.dylib on recent macOS) mixes struct layouts and
# crashes, so the core and extensions are compiled into the bundle.
vendor_cmark_src = File.join(vendor_cmark, 'src')
vendor_cmark_ext = File.join(vendor_cmark, 'extensions')
unless File.exist?(File.join(vendor_cmark_src, 'blocks.c')) &&
       File.exist?(File.join(vendor_cmark_ext, 'table.c'))
  abort "apex-ruby: bundled cmark-gfm sources are missing from #{vendor_cmark}. " \
        'Run `git submodule update --init --recursive` in the apex-ruby checkout.'
end
$INCFLAGS << " -I#{vendor_cmark_src}"
$INCFLAGS << " -I#{vendor_cmark_ext}"
$defs << ' -DCMARK_GFM_STATIC_DEFINE -DCMARK_GFM_EXTENSIONS_STATIC_DEFINE'
$defs << ' -DHAVE_STDBOOL_H=1 -DHAVE___BUILTIN_EXPECT=1 -DHAVE___ATTRIBUTE__=1'

# cmark-gfm's buffer.c and utf8.c share basenames with Apex sources, and mkmf
# builds every object into one directory, so each cmark-gfm source is compiled
# through a uniquely named wrapper.
cmark_sources = Dir[File.join(vendor_cmark_src, '*.c')].reject { |f| File.basename(f) == 'main.c' }
cmark_wrappers = cmark_sources.map { |f| ['cmark_', f] } +
                 Dir[File.join(vendor_cmark_ext, '*.c')].map { |f| ['cmark_ext_', f] }
cmark_wrapper_names = cmark_wrappers.map do |prefix, path|
  name = "#{prefix}#{File.basename(path)}"
  File.write(name, "#include \"#{path}\"\n")
  name
end

# Add Apex source tree (core + extensions) to VPATH so we can refer to
# files by basename and let make find the sources.
$VPATH << File::PATH_SEPARATOR << apex_srcdir
$VPATH << File::PATH_SEPARATOR << apex_extdir

core_sources = Dir[File.join(apex_srcdir, '*.c')]
ext_sources  = Dir[File.join(apex_extdir, '*.c')]

$srcs = core_sources.map { |f| File.basename(f) } +
        ext_sources.map { |f| File.basename(f) } +
        cmark_wrapper_names +
        ['apex_ext.c']

# ---- libyaml (optional, enables nested YAML front matter) -----------------
have_libyaml = false
if pkg_config('yaml-0.1') || pkg_config('yaml')
  have_libyaml = true
elsif have_header('yaml.h') && have_library('yaml')
  have_libyaml = true
else
  # Homebrew libyaml without pkg-config in some setups
  brew_yaml = `brew --prefix libyaml 2>/dev/null`.strip
  unless brew_yaml.empty?
    yaml_inc = File.join(brew_yaml, 'include')
    yaml_lib = File.join(brew_yaml, 'lib')
    if File.exist?(File.join(yaml_inc, 'yaml.h'))
      $INCFLAGS << " -I#{yaml_inc}"
      $LDFLAGS << " -L#{yaml_lib}"
      if have_library('yaml')
        have_libyaml = true
        if RbConfig::CONFIG['host_os'].to_s.include?('darwin')
          $LDFLAGS << " -Wl,-rpath,#{yaml_lib}"
        end
      end
    end
  end
end

if have_libyaml
  $defs << ' -DAPEX_HAVE_LIBYAML'
  puts 'libyaml: enabled (nested YAML front matter)'
else
  puts 'libyaml: not found (using Apex simple YAML front-matter parser)'
end

# Apex sources include "extensions/...." from the src/ tree
$INCFLAGS << " -I#{apex_srcdir}"

create_makefile('apex_ext/apex_ext')
