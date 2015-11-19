import gulp from 'gulp';
import gulpLoadPlugins from 'gulp-load-plugins';
import minimist from 'minimist';
import requireDir from 'require-dir';
import del from 'del';
import mkdirp from 'mkdirp';

const knownOptions = {
  string: 'env',
  default: { env: process.env.NODE_ENV || 'development' }
};

const plugins = gulpLoadPlugins();
const src = Object.create(null);
const options = minimist(process.argv.slice(2), knownOptions);

const config = require('./gulp/config/' + options.env + '.json');
config.options = options;
config.gulp = { watch: false };

const tasks = requireDir('./gulp/tasks');
for (let taskName of Object.keys(tasks)) {
  tasks[taskName](plugins, src, config);
}

gulp.task('default', ['build']);

// Clean output directory
gulp.task('clean', cb => {
  del(['.tmp', 'build/*', '!build/.git'], {dot: true}).then(() => {
    mkdirp('build/public', cb);
  });
});
