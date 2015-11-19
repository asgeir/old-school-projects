import _ from 'lodash';
import elasticsearch from 'elasticsearch';
import {initSearch as initCompanySearch} from './models/company';

export let client = null;

export function initSearch(connectionString, searchOptions) {
  client = new elasticsearch.Client(_.merge({ host: connectionString }, searchOptions));

  initCompanySearch();
}
