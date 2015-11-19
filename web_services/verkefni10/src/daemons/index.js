import {initialize as initializeTokenDaemon} from './token_daemon';

export function initDaemons(config) {
  initializeTokenDaemon(config);
}
