import QtQuick 2.0

Canvas {
    id: canvas
    property real startX: 0
    property real startY: 0
    property real endX: width
    property real endY: height
    property bool dashed: false
    property real dashLength: 10
    property real dashSpace: 8
    property real lineWidth: 1
    property real stippleLength: (dashLength + dashSpace) > 0 ? (dashLength + dashSpace) : 16
    property color color: "lightgray"
    onPaint: {
        // Get the drawing context
        var ctx = canvas.getContext('2d')
        // set line color
        ctx.strokeStyle = color;
        ctx.lineWidth = lineWidth;
        ctx.beginPath();

        if (!dashed)
        {
            ctx.moveTo(startX,startY);
            ctx.lineTo(endX,endY);
        }
        else
        {
            var dashLen = stippleLength;
            var dX = endX - startX;
            var dY = endY - startY;
            var dashes = Math.floor(Math.sqrt(dX * dX + dY * dY) / dashLen);
            if (dashes == 0)
            {
                dashes = 1;
            }
            var dash_to_length = dashLength/dashLen
            var space_to_length = 1 - dash_to_length
            var dashX = dX / dashes;
            var dashY = dY / dashes;
            var x1 = startX;
            var y1 = startY;

            ctx.moveTo(x1,y1);

            var q = 0;
            while (q++ < dashes) {
                x1 += dashX*dash_to_length;
                y1 += dashY*dash_to_length;
                ctx.lineTo(x1, y1);
                x1 += dashX*space_to_length;
                y1 += dashY*space_to_length;
                ctx.moveTo(x1, y1);
            }
        }
        ctx.stroke();
    }
}
