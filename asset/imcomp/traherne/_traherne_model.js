function _traherne_model() {
  this.via = {};

  this.files = {};
  this.fid_to_index = {};
  this.index_to_fid = {};

  this.base = {};
  this.comp = {};

}

_traherne_model.prototype.init = function( traherne_controller ) {
  this.c = traherne_controller;

  for( var type in this.c.type_list ) {
    var via_panel = document.getElementById( type + '_via_panel' );
    this.via[type] = new _via();
    this.via[type].init(via_panel);
  }
}

_traherne_model.prototype.add_images = function( type, files ) {
  this.files[type] = Array.from(files);

  this.fid_to_index[type] = {};
  this.index_to_fid[type] = {};

  // sort files based on filename
  this.files[type].sort(
    function(a,b) {
      if( a.name < b.name ) {
        return -1;
      }
      if( a.name > b.name ) {
        return 1;
      }
      return 0;
    }
  );

  var n = this.files[type].length;
  var promises = [];
  for (var i=0; i < n; i++) {
    console.log('Adding local file to ' + type + ' : ' + this.files[type][i].name);
    promises.push( this.via[type].m.add_file_local(this.files[type][i]) );
  }

  Promise.all(promises).then( function(result) {
    var n = result.length;
    for( var i=0; i<n; i++ ) {
      var fid = result[i];
      if ( !fid.startsWith("Error") ) {
        // record the internal file-id in via
        this.fid_to_index[type][fid] = i;
        this.index_to_fid[type][i] = fid;
      }
    }
    this.c.on_filelist_update(type);
  }.bind(this));
}

_traherne_model.prototype.get_filelist = function(type) {
  var filelist = [];
  var n = this.files[type].length;

  for( var i=0; i<n; i++ ) {
    filelist.push(this.files[type][i].name);
  }
  return filelist;
}
