import gulp from 'gulp';

export default (plugin, src, config) => {
  // Launch a Node.js/Express server
  gulp.task('serve', ['build:watch'], () => {
    src.server = [
      'build/server.js',
      'build/templates/**/*'
    ];
    console.log('hey');
    plugin.nodemon({
      script: 'build/server.js',
      env: Object.assign({NODE_ENV: 'development'}, process.env),
      watch: src.server
    });
  });
};
