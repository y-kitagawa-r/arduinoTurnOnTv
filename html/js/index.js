(function () {
$(function(){
  init();
});

function init() {
  var clickCounter = 0;
  $('#button').click(function(e){
    clickCounter++;
    sendStatus(clickCounter);
  });
}

function sendStatus(clickCounter){
  $.ajax({
    type: "post",
    url: "statusSet.php",
    dataType: "json",
    data:{
      status:clickCounter
    }
  }).done(function(data){
    console.log("success " + data);// 確認メッセージ用
  }).fail(function(jqXHR, textStatus, errorThrown){
    console.log("error " + data);// 確認メッセージ用
  });
}

})();
