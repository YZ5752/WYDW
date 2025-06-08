/**
 * 自定义Cesium InfoBox组件
 * 该文件修改默认InfoBox行为，使其只显示摄像机聚焦按钮
 */

// 在Cesium完全加载后执行
document.addEventListener('DOMContentLoaded', function() {
  // 确保Cesium已加载
  if (typeof Cesium !== 'undefined') {
    // 创建样式元素
    const styleElement = document.createElement('style');
    styleElement.type = 'text/css';
    styleElement.textContent = `
      /* 隐藏InfoBox除摄像机按钮外的内容 */
      .cesium-infoBox {
        background: transparent !important;
        border: none !important;
        max-width: 44px !important;
        width: 44px !important;
        height: 44px !important;
        top: 65px !important;
        pointer-events: none !important;
      }
      
      /* 确保摄像机按钮可点击 */
      button.cesium-infoBox-camera {
        background: rgba(38, 38, 38, 0.9) !important;
        border: 1px solid #444 !important;
        border-radius: 5px !important;
        pointer-events: auto !important;
        width: 32px !important;
        height: 32px !important;
        left: 0 !important;
        top: 0 !important;
      }
      
      /* 隐藏InfoBox其他部分 */
      .cesium-infoBox-title, 
      .cesium-infoBox-close, 
      .cesium-infoBox-iframe,
      .cesium-infoBox-description {
        display: none !important;
      }
      
      /* 确保按钮在选择时才显示 */
      .cesium-infoBox:not(.cesium-infoBox-visible) .cesium-infoBox-camera {
        display: none !important;
      }
    `;

    // 将样式添加到文档头部
    document.head.appendChild(styleElement);

    // 原始Viewer.prototype._onTick函数的引用
    const originalOnTick = Cesium.Viewer.prototype._onTick;
    
    // 重写_onTick函数，确保当选择实体时启用摄像机按钮
    Cesium.Viewer.prototype._onTick = function(e) {
      // 调用原始函数
      originalOnTick.call(this, e);
      
      // 确保选择了实体后启用摄像机按钮
      if (this.selectedEntity && this._infoBox && this._infoBox.viewModel) {
        // 强制启用摄像机按钮，即使实体没有position属性
        this._infoBox.viewModel.enableCamera = true;
        
        // 确保InfoBox可见
        this._infoBox.viewModel.showInfo = true;
      }
    };
    
    // 确保_onInfoBoxCameraClicked始终尝试使用zoomTo而不是必须有position属性
    const originalCameraClicked = Cesium.Viewer.prototype._onInfoBoxCameraClicked;
    Cesium.Viewer.prototype._onInfoBoxCameraClicked = function(e) {
      if (e.isCameraTracking && this.trackedEntity === this.selectedEntity) {
        this.trackedEntity = void 0;
      } else {
        // 始终尝试zoomTo选中的实体，不要检查position属性
        this.zoomTo(this.selectedEntity);
      }
    };
    
    console.log("自定义InfoBox样式已应用");
  } else {
    console.error("Cesium未加载，无法修改InfoBox");
  }
}); 