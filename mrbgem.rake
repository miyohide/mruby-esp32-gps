MRuby::Gem::Specification.new('mruby-esp32-gps') do |spec|
  spec.icense = 'MIT'
  spec.authors = 'Hidenori Miyoshi'
  spec.cc.include_paths << "#{build.root}/src"
end

