var common =
{
    enemyOnCollide : function(ship, missile)
    {
        missile.dead = true;

        this.shields -= missile.energy;
        if (this.shields <= 0)
        {
            this.dead = true;        
            ship.score += 10;
            engine.data.state.stats.enemiesDestroyed++;
        }
    },
};

var Enemy = function () {
    this.width = 16;
    this.height = 24;
    this.x = 256 + 224 * (Math.random() - .5);
    this.y = -2000 * Math.random() - 288;
    this.shields = 10;

    this.image = new Image();
    this.image.src = "enemy16x24.png";
};
$.extend(Enemy.prototype, Entity.prototype);
$.extend(Enemy.prototype, {

    collidable: true,
    oncollide : common.enemyOnCollide,

    update: function (gtime) {
        this.y += 2;
        if (this.y > 288 + this.height) {
            this.dead = true;
            engine.data.state.stats.enemiesEscaped++;
        }
    },

    draw: function (ctx, gtime) {
        ctx.fillStyle = "cyan";
        ctx.drawImage(this.image, this.x - this.width / 2, this.y - this.height / 2);
    }

});

var Enemy2 = defineClass(Entity,
    function (options) {

        options = $.extend({
            width : 48,
            height : 39,
            x : 256 + 224 * (Math.random() - .5),
            y : -39,
            shields : 100,
        }, options);

        var _this = this;
        lx.core.each(options, function(key,value) {
            _this[key] = value;
        });        

        this.image = new Image();
        this.image.src = "enemy-2-48x39.png";
    },
    {
        collidable: true,
        oncollide : common.enemyOnCollide,

        update: function (gtime) {
            this.y += .75;
            if (this.y > 288 + this.height) {
                this.dead = true;
                engine.data.state.stats.enemiesEscaped++;
            }
        },

        draw: function (ctx, gtime) {
            ctx.drawImage(this.image, this.x - this.width / 2, this.y - this.height / 2);
        }
    }
);

