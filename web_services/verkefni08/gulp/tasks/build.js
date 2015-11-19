import gulp from 'gulp';
import runSequence from 'run-sequence';

export default (plugin, src, config) => {
  // Static files
  gulp.task('assets', () => {
    src.assets = 'assets/static/**';
    return gulp.src(src.assets)
      .pipe(plugin.changed('build/public'))
      .pipe(gulp.dest('build/public'))
      .pipe(plugin.size({title: 'assets'}));
  });

  // Resource files
  gulp.task('resources', () => {
    src.resources = [
      'package.json',
      'assets/templates*/**'
    ];
    return gulp.src(src.resources)
      .pipe(plugin.changed('build'))
      .pipe(gulp.dest('build'))
      .pipe(plugin.size({title: 'resources'}));
  });

  // Build the app from source code
  gulp.task('build', ['clean'], taskDone => {
    runSequence(['assets', 'resources'], ['bundle'], taskDone);
  });

  // Build and start watching for modifications
  gulp.task('build:watch', taskDone => {
    config.gulp.watch = true;
    runSequence('build', () => {
      gulp.watch(src.assets, ['assets']);
      gulp.watch(src.resources, ['resources']);
      taskDone();
    });
  });
};
