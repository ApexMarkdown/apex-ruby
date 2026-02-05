require "rake/clean"

require_relative 'lib/apex/version'

PKG_DIR   = File.expand_path('pkg', __dir__)
GEM_NAME  = "apex-#{Apex::VERSION}.gem"
GEM_PATH  = File.join(PKG_DIR, GEM_NAME)

directory PKG_DIR

CLEAN.include('ext/**/Makefile', 'ext/**/*.o', 'ext/**/*.so', 'ext/**/*.bundle')
CLOBBER.include(PKG_DIR)

desc 'Compile native extension'
task :compile do
  Dir.chdir(File.join(__dir__, 'ext', 'apex_ext')) do
    ruby 'extconf.rb'
    sh 'make'
  end
end

desc 'Run test suite'
task test: :compile do
  cmark_prefix = `brew --prefix cmark-gfm 2>/dev/null`.strip
  env = ENV.to_h
  unless cmark_prefix.empty?
    lib = "#{cmark_prefix}/lib"
    env['DYLD_LIBRARY_PATH'] = [lib, ENV['DYLD_LIBRARY_PATH']].compact.join(File::PATH_SEPARATOR)
  end
  sh env, 'ruby -Itest -rminitest/autorun test/test_apex.rb'
end

task default: :test

desc 'Build gem package'
task package: PKG_DIR do
  sh 'gem build apex.gemspec'
  mv GEM_NAME, PKG_DIR
end

namespace :bump do
  def update_version(segment)
    version_file = File.join(__dir__, 'lib', 'apex', 'version.rb')
    content = File.read(version_file)
    current = Apex::VERSION.split('.').map!(&:to_i)

    case segment
    when :major
      current[0] += 1
      current[1] = 0
      current[2] = 0
    when :minor
      current[1] += 1
      current[2] = 0
    when :patch
      current[2] += 1
    end

    new_version = current.join('.')
    content.sub!(/VERSION = "\d+\.\d+\.\d+"/, %(VERSION = "#{new_version}"))
    File.write(version_file, content)

    puts "Bumped version to #{new_version}"
  end

  desc 'Bump patch version'
  task :patch do
    update_version(:patch)
  end

  desc 'Bump minor version'
  task :minor do
    update_version(:minor)
  end

  desc 'Bump major version'
  task :major do
    update_version(:major)
  end
end

desc 'Install built gem into all mise Ruby versions'
task install: :package do
  abort 'mise not found in PATH; cannot install to multiple Ruby versions' unless system('which mise > /dev/null 2>&1')

  versions_output = `mise ls ruby`
  versions = versions_output.lines
                            .map(&:strip)
                            .reject { |l| l.empty? || l.start_with?('#') }

  abort "No Ruby versions reported by 'mise ls ruby'" if versions.empty?

  versions.each do |ver|
    cmd = ['mise', 'exec', "ruby@#{ver}", '--', 'gem', 'install', GEM_PATH]
    puts "Installing to ruby@#{ver}..."
    system(*cmd) || warn("Failed to install to ruby@#{ver}")
  end
end
