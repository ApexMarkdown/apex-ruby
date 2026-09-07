module Apex
  # Apex Ruby binding version.
  #
  # This is the version of the Ruby gem, not the underlying
  # Apex C library. The C library version can be queried
  # via Apex::Native.version.
  module Version
    # Current gem version.
    VERSION = "1.0.19"
  end

  # Shortcut to gem version string.
  VERSION = Version::VERSION
end

