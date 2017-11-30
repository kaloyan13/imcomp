function _traherne_controller() {
  this.state = {};
  this.now = {};
  this.now.base = {};
  this.now.comp = {};
}

_traherne_controller.prototype.init = function( traherne_model, traherne_view ) {
  this.m = traherne_model;
  this.v = traherne_view;
}

_traherne_controller.prototype.update_files = function(type, e) {
  console.log(type);
  console.log(e);
  this.m.add_images(type, e.target.files);
}

_traherne_controller.prototype.move_to_next = function(type) {
  var n = this.m.files[type].length;
  if( n > 0 ) {
    var now_findex = this.v.now[type].findex;
    if( now_findex === (n - 1) ) {
      this.set_now(type, 0);
    } else {
      this.set_now(type, now_findex + 1);
    }
  }
}

_traherne_controller.prototype.move_to_prev = function(type) {
  var n = this.m.files[type].length;
  if( n > 0 ) {
    var now_findex = this.v.now[type].findex;
    if( now_findex === 0 ) {
      this.set_now(type, n - 1);
    } else {
      this.set_now(type, now_findex - 1);
    }
  }
}


_traherne_controller.prototype.on_filelist_update = function(type) {
  var filelist = this.m.get_filelist(type);
  this.update_view_filelist(type, filelist);
  this.set_now(type, 0); // show first image

}

_traherne_controller.prototype.update_view_filelist = function(type, filelist) {
  var list_name = type + '_img_filename_list';
  var list = document.getElementById(list_name);
  var n = filelist.length;

  for( var i=0; i<n; i++ ) {
    var opt = document.createElement('option');
    opt.value = i;
    opt.text = '[' + (i+1) + '] ' + filelist[i];
    list.add(opt, null);
  }
}

_traherne_controller.prototype.set_now = function(type, findex) {
  if(type === 'base') {
    this.m.via1.c.load_file_from_index( findex );
    this.v.now[type].findex = findex;
  } else {
    console.log('@todo for comp');
  }
  this.on_now_update(type, findex);
}

_traherne_controller.prototype.on_now_update = function(type, findex) {
  // update the selected item in filelist dropdown
  var list_name = type + '_img_filename_list';
  var list = document.getElementById(list_name);
  var now_findex = this.v.now[type].findex;

  if( list.options.length ) {
    for ( var i=0; i<list.options.length; i++ ) {
      if ( list.options[i].value == findex ) {
        list.options[i].setAttribute('selected', 'selected');
      } else {
        if ( list.options[i].hasAttribute('selected') ) {
          list.options[i].removeAttribute('selected')
        }
      }
    }
  }
}

_traherne_controller.prototype.base_now_update = function(e) {
  var findex = e.target.value;
  this.set_now('base', findex);
}

_traherne_controller.prototype.comp_now_update = function(e) {
  var findex = e.target.value;
  this.set_now('comp', findex);
}
