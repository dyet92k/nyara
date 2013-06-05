require "mkmf"

def build_prereq
  puts "building multipart-parser-c"
  dir = File.dirname __FILE__
  multipart_dir = File.join dir, "multipart-parser-c"
  Dir.chdir multipart_dir do
    system "make"
  end

  puts "building http-parser"
  http_parser_dir = File.join dir, "http-parser"
  Dir.chdir http_parser_dir do
    system "make libhttp_parser.o"
  end

  flags = " -I#{multipart_dir.shellescape} -I#{http_parser_dir.shellescape}"
  $CFLAGS << flags
  $CPPFLAGS << flags
end

def tweak_cflags
  mf_conf = RbConfig::MAKEFILE_CONFIG
  # enable c++11 (todo $CPPFLAGS ?)
  mf_conf['CXXFLAGS'] << ' -stdlib=libc++ -std=c++11'

  $CFLAGS << ' $(xflags)'
  puts 'To add extra CFLAGS:'
  puts "  make xflags='-DNDEBUG -O0'"
end

def modify_makefile
  puts "modifying Makefile"
  makefile = File.readlines 'Makefile'
  makefile.each do |line|
    if line.start_with?('OBJS =')
      line.sub!(/$/, " http-parser/libhttp_parser.o multipart-parser-c/multipart_parser.o")
      break
    end
  end
  File.open 'Makefile', 'w' do |f|
    f.puts makefile
  end
end

build_prereq
tweak_cflags
create_makefile 'nyara'
modify_makefile
