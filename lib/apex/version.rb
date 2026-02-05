module Apex
  # Apex Ruby binding version.
  #
  # This is the version of the Ruby gem, not the underlying
  # Apex C library. The C library version can be queried
  # via the C API once bound.
  module Version
    # Current gem version.
    VERSION = "1.0.6"
  end

  # Shortcut to gem version string.
  VERSION = Version::VERSION
end

