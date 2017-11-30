function ImageComparator() {
  this.m = new _imcomp_model();  // model
  this.v = new _imcomp_view();   // view
  this.c = new _imcomp_ctrl();   // controller

  this.init = function() {
    console.log("Initializing Image Comparator ...");
    this.m.init( this.c );
    this.v.init( this.c );
    this.c.init( this.m, this.v );
  }
}

var toggle_timer = 0;

var _imcomp = new ImageComparator();
_imcomp.init();
