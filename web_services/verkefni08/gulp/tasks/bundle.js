import gulp from 'gulp';
import webpack from 'webpack';

export default (plugin, src, config) => {
  // Bundle
  gulp.task('bundle', taskDone => {
    const webpackConfig = require('../../webpack.config.js');
    const bundler = webpack(webpackConfig);
    const verbose = !!config.options.verbose;
    let bundlerRunCount = 0;

    function bundle(err, stats) {
      if (err) {
        throw new plugin.util.PluginError('webpack', err);
      }

      console.log(stats.toString({
        colors: plugin.util.colors.supportsColor,
        hash: verbose,
        version: verbose,
        timings: verbose,
        chunks: verbose,
        chunkModules: verbose,
        cached: verbose,
        cachedAssets: verbose
      }));

      console.log('bundled ' + webpackConfig.length);
      if (++bundlerRunCount === (config.gulp.watch ? webpackConfig.length : 1)) {
        return taskDone();
      }
    }

    if (config.gulp.watch) {
      bundler.watch(200, bundle);
    } else {
      bundler.run(bundle);
    }
  });
};
