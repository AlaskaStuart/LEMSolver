## 读取数据格式

- 使用 json 格式保存计算所需剖面数据

- 整体框架如下所示：

```mermaid
  %%{init: {'theme': 'neutral','look':'handDrawn'}}%%
  flowchart TD
      A["utils.h"] --> B["model.h"]
      A --> C["geometry.h"]
      F["settings.h"] -->C
      B --> D["algorithm.h"]
      F["settings.h"] -->D
      C --> D
      D -->E["sovler.h"]
      F["settings.h"] -->E

```

## 条分法的实现逻辑

- 在插值时生成 n+2 个点(包含入口点和出口点), 切条的宽度为 1/(n+1), 如果不包含入口点和出口点, 个数为 slicesNum, 因此在插值时的插值点数量为 slicesNum+2
- 所有的点偏移半个 dx 宽度,因此返回插值结果时, 移除最后一个(出口点), 因为越界了
- 最终生成的条带数量为 n+1 个
- 最后在计算安全系数时, 因为要考虑条带底部的斜率, 条带与紧邻的下一个条带的斜率最为本条带的底边斜率, 此时最后一个条带无法形成斜率, 因此舍去掉最后一个条带, 实际参与运算的条带数量为 n 个

## Notes

- 关于`getUpperPolyLineIntersection`方法一开始设想的是，写成重载方法，但是针对多线段的情况，如果交点不止一个，那么最后一个交点的右侧（尾段）该怎么处理，没有特别想好；再有，如果两个不相交，那么怎么判断哪条线段在上，哪条线段在下

## 参考资料

- https://help.geostru.eu/slope/en/index.html
- https://www.geoengineer.org/education/slope-stability/slope-stability-introduction-to-the-method-of-slices
- https://www.geoengineer.org/education/slope-stability/slope-stability-the-bishop-method-of-slices
- https://www.geoengineer.org/education/slope-stability/slope-stability-the-janbu-method
- https://files.seequent.com/PDFs/Geostudio-Stability%20Modeling-Oct2022.pdf
