require "mkmf"

apex_src      = File.expand_path('apex_src', __dir__)
apex_incl     = File.join(apex_src, 'include')
apex_srcdir   = File.join(apex_src, 'src')
apex_extdir   = File.join(apex_srcdir, 'extensions')
vendor_cmark  = File.join(apex_src, 'vendor', 'cmark-gfm')
config_incl_dir = File.join(apex_src, 'PackageSupport', 'cmark-gfm')
$INCFLAGS << " -I#{apex_incl}"
$INCFLAGS << " -I#{config_incl_dir}"

# html_renderer.c includes internal cmark-gfm headers like "table.h" which
# are not installed by system cmark-gfm. When the Apex submodule has its
# cmark-gfm vendor populated, add that header path so those includes work,
# while still linking against the system library.
vendor_cmark_src = File.join(vendor_cmark, 'src')
vendor_cmark_ext = File.join(vendor_cmark, 'extensions')
$INCFLAGS << " -I#{vendor_cmark_src}" if Dir.exist?(vendor_cmark_src)
$INCFLAGS << " -I#{vendor_cmark_ext}" if Dir.exist?(vendor_cmark_ext)

# Add Apex source tree (core + extensions) to VPATH so we can refer to
# files by basename and let make find the sources.
$VPATH << File::PATH_SEPARATOR << apex_srcdir
$VPATH << File::PATH_SEPARATOR << apex_extdir

core_sources = Dir[File.join(apex_srcdir, '*.c')]
ext_sources  = Dir[File.join(apex_extdir, '*.c')]

# Apex's html_renderer.c uses CMARK_NODE_* from cmark-gfm extensions (table,
# strikethrough, etc.). The system libcmark-gfm often does not export these;
# compile all vendored extension .c files into the bundle so the symbols exist.
cmark_ext_dir = File.join(vendor_cmark, 'extensions')
cmark_ext_sources = Dir[File.join(cmark_ext_dir, '*.c')]
if cmark_ext_sources.any?
  $VPATH << File::PATH_SEPARATOR << cmark_ext_dir
  cmark_ext_basenames = cmark_ext_sources.map { |f| File.basename(f) }
  $srcs = core_sources.map { |f| File.basename(f) } +
          ext_sources.map { |f| File.basename(f) } +
          cmark_ext_basenames +
          ['apex_ext.c']
else
  $srcs = core_sources.map { |f| File.basename(f) } +
          ext_sources.map { |f| File.basename(f) } +
          ['apex_ext.c']
end

# ---- cmark-gfm system detection ------------------------------------------
#
# We link against a system cmark-gfm installation, using the vendored
# headers (if present) for internal types like CMARK_NODE_TABLE.
#
# Set APEX_SHOW_CMARK_ERROR=1 to print the "cmark-gfm not found" message and
# exit without building (useful for previewing the user-facing error).
#
CMARK_NOT_FOUND_MSG = <<~MSG
  -------------------------------------------------------------------------------
  The apex gem could not find the cmark-gfm C library.
  -------------------------------------------------------------------------------

  This gem requires cmark-gfm to be installed before you can build the native
  extension. Install it for your system, then run `gem install apex` again.

  macOS (Homebrew):
    brew install cmark-gfm

  Other platforms:
    Install the cmark-gfm development package for your distribution, ensure
    pkg-config is available, then retry. See:
    https://github.com/github/cmark-gfm

  -------------------------------------------------------------------------------
MSG

def require_cmark_gfm!
  if ENV['APEX_SHOW_CMARK_ERROR']
    warn CMARK_NOT_FOUND_MSG
    abort
  end

  if pkg_config('cmark-gfm')
    # pkg-config has already added the right flags (and usually rpath).
    return true
  end

  have_header('cmark-gfm-core-extensions.h')
  unless have_library('cmark-gfm')
    abort CMARK_NOT_FOUND_MSG
  end

  # Embed rpath so the .bundle finds libcmark-gfm at load time (macOS often
  # ignores DYLD_LIBRARY_PATH for dlopen'd dependencies).
  cmark_lib = `brew --prefix cmark-gfm 2>/dev/null`.strip
  cmark_lib = File.join(cmark_lib, 'lib') if !cmark_lib.empty? && Dir.exist?(File.join(cmark_lib, 'lib'))
  if RbConfig::CONFIG['host_os'].to_s.include?('darwin') && !cmark_lib.empty?
    $LDFLAGS << " -Wl,-rpath,#{cmark_lib}"
  end

  true
end

require_cmark_gfm!

create_makefile('apex_ext/apex_ext')
