<?php
header("Content-type: application/json; charset=utf8");
if(isset($_POST['status'])){
  $status = $_POST['status'];
  file_put_contents("statusLog.txt","status:".$status);
  echo json_encode($status);

}else{
  $error = "失敗";
  echo json_encode($error);
}

?>
