function _traherne_model() {
  this.via1 = new _via(); // for base

  this.files = {};
  this.fid_to_index = {};
  this.index_to_fid = {};

  this.base = {};
  this.comp = {};
}

_traherne_model.prototype.init = function( traherne_controller ) {
  this.c = traherne_controller;

  var via_panel = document.getElementById('left_via_panel');
  this.via1.init(via_panel);
}

_traherne_model.prototype.add_images = function( type, files ) {
  this.files[type] = Array.from(files);

  this.fid_to_index[type] = {};
  this.index_to_fid[type] = {};

  // sort files based on filename
  console.log('sorting');
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
    console.log('Adding local file : ' + this.files[type][i].name);
    promises.push( this.via1.m.add_file_local(this.files[type][i]) );
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
    console.log(this.fid_to_index);
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
