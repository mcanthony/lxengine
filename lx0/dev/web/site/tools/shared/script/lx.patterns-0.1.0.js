try{eval("lx")}catch(g){lx={}}lx.patterns||(lx.patterns={});
(function(b){b.checker=function(a){a=_lxbb_abs_vec2(_lxbb_fract_vec2(a));a=_lxbb_floor_vec2(_lxbb_mul_vec2_float(a,2));return Math.floor(a[0]+a[1])%2};b.spot=function(a){a=_lxbb_sub_vec2([0.5,0.5],_lxbb_fract_vec2(a));return _lxbb_lengthSqrd_vec2(a)&lt;0.4*0.4?1:0};b.stripe=function(a){return 0.3&gt;Math.abs(0.5-_lxbb_fract_float(a[0]))?1:0};b.tile=function(a){a=_lxbb_abs_vec2(_lxbb_sub_vec2([0.5,0.5],_lxbb_fract_vec2(a)));return 0.45&gt;a[0]&&0.45&gt;a[1]?1:0};b.diamond=function(a){a=_lxbb_abs_vec2(_lxbb_sub_vec2([0.5,
0.5],_lxbb_fract_vec2(a)));return 0.4&gt;=a[0]+a[1]?1:0};b.wave=function(a){a=_lxbb_fract_vec2(a);a[0]+=0.15*Math.sin(2*a[1]*Math.PI);a[0]=Math.abs(0.5-a[0]);return 0.25&gt;a[0]?1:0};b.circle=function(a){a=_lxbb_sub_vec2([0.5,0.5],_lxbb_fract_vec2(a));a=_lxbb_lengthSqrd_vec2(a);return a&lt;0.4*0.4&&0.09&lt;a?1:0};b.roundedTile=function(a){var e=_lxbb_sub_vec2([0.5,0.5],_lxbb_fract_vec2(a)),e=0.25&gt;_lxbb_lengthSqrd_vec2(e)?1:0;return b.b(a)*e};b.ribbon=function(a){a=_lxbb_fract_vec2(a);a[1]+=0.15*Math.sin(2*a[0]*
Math.PI);a=_lxbb_abs_vec2(_lxbb_sub_vec2([0.5,0.5],a));return a[1]&lt;0.15*(a[0]+0.1)?0:1};b.spotwave=function(a){return b.a(a)*b.d(a)};b.spotdiamondxor=function(a){return 1!=b.a(_lxbb_mul_vec2_float(a,2))+b.c(a)?1:0};b.star=function(a){a=_lxbb_abs_vec2(_lxbb_sub_vec2([0.5,0.5],_lxbb_fract_vec2(a)));a=_lxbb_log_vec2(_lxbb_mul_float_vec2(Math.E,_lxbb_abs_vec2(_lxbb_add_vec2(a,[0.5,0.5]))));return a[0]*a[1]&gt;0.52*0.52?0:1};b.misc1=function(a){a=_lxbb_add_vec2(_lxbb_abs_vec2(_lxbb_sub_vec2([0.5,0.5],_lxbb_fract_vec2(a))),
[0.5,0.5]);a[0]=Math.pow(a[0],0.7);a[1]=Math.pow(a[1],1.5);return a[0]*a[1]&gt;0.65*Math.cos(a[0])?0:1};b.test=function(a){return b.b(_lxbb_add_vec2(a,[0.5,0.5]))||b.a(_lxbb_mul_vec2_float(a,8))};b.weave=function(a){var a=_lxbb_mul_float_vec2(2,a),b=_lxbb_fract_vec2(a),a=_lxbb_sub_vec2(a,b),b=_lxbb_abs_vec2(_lxbb_sub_vec2([0.5,0.5],b)),d=_lxbb_add_vec2([0.5,0.5],_lxbb_div_vec2_float(_lxbb_pow_vec2_float(_lxbb_cos_vec2(_lxbb_mul_vec2_float(b,Math.PI)),0.75),2)),c=0&lt;(a[0]+a[1])%2?1:0,f=1-c,a=d[c],d=d[f],
c=b[c],b=b[f];return 0.4&gt;c?d:0.4&gt;b?1-a:0}})(lx.patterns);