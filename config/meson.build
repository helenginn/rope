conf = configuration_data()

conf.set('datadir', data_dir)
conf.set('version', meson.project_version())

cfile = configure_file(input : 'config.h.in',
  output : 'config.h',
  configuration : conf)
