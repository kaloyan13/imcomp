function traherne_imcomp_user_interface() {
  // see https://developer.apple.com/library/content/documentation/General/Conceptual/DevPedia-CocoaCore/MVC.html
  this.m = new _traherne_model();
  this.v = new _traherne_view();
  this.c = new _traherne_controller();

  // name, version, etc defined in _model.js

  this.init = function() {
    console.log("Initializing Traherne user interface components ...");
    this.m.init( this.c );
    this.v.init( this.c );
    this.c.init( this.m, this.v );


  }
}

var _traherne_about = {};
_traherne_about.name = "Traherne Digital Collator";
_traherne_about.shortname = "traherne";
_traherne_about.version = "2.0.1";
_traherne_about.date_first_release  = "June 01, 2017";
_traherne_about.date_current_release  = "";
_traherne_about.author_name  = "Abhishek Dutta";
_traherne_about.author_email = "adutta@robots.ox.ac.uk";

var _traherne_toggle_timer = {};
var _traherne_compare_button_anim_timer = {};

var _traherne_imcomp_user_interface = new traherne_imcomp_user_interface();
_traherne_imcomp_user_interface.init();
