import path from 'path';
import webpack, { BannerPlugin, DefinePlugin } from 'webpack';
import minimist from 'minimist';
import {merge} from 'lodash';

const argv = minimist(process.argv.slice(2));
const DEBUG = !argv.release;
const GLOBALS = {
  'process.env.NODE_ENV': DEBUG ? '"development"' : '"production"',
  '__DEV__': DEBUG
};

const config = {
  entry: './src/main.js',
  output: {
    path: './build',
    filename: 'server.js',
    publicPath: '/static/',
    sourcePrefix: '  ',
    libraryTarget: 'commonjs2'
  },

  target: 'node',
  externals: /^[a-z][a-z\.\-0-9]*$/,
  node: {
    console: false,
    global: false,
    process: false,
    Buffer: false,
    __filename: false,
    __dirname: false
  },

  cache: DEBUG,
  debug: DEBUG,

  stats: {
    colors: true,
    reasons: DEBUG
  },

  plugins: [
    new webpack.optimize.OccurenceOrderPlugin(),
    new DefinePlugin(merge(GLOBALS, {'__SERVER__': true})),
    new BannerPlugin('require("source-map-support").install();', { raw: true, entryOnly: false })
  ],

  resolve: {
    alias: {
      'static-assets': (path.resolve(__dirname, './assets'))
    },
    extensions: ['', '.webpack.js', '.web.js', '.js', '.jsx']
  },

  module: {
    preLoaders: [
      {
        test: /\.js$/,
        exclude: /node_modules/,
        loader: 'eslint-loader'
      }
    ],

    loaders: [
      {
        test: /\.js$/,
        exclude: /node_modules/,
        loader: 'babel-loader'
      }
    ]
  }
};

export default [config];
