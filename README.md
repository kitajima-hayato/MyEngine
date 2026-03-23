[![DebugBuild](https://github.com/kitajima-hayato/MyEngine/actions/workflows/DebugBuild.yml/badge.svg)](https://github.com/kitajima-hayato/MyEngine/actions/workflows/DebugBuild.yml)


[![ReleaseBuild](https://github.com/kitajima-hayato/MyEngine/actions/workflows/ReleaseBuild.yml/badge.svg)](https://github.com/kitajima-hayato/MyEngine/actions/workflows/ReleaseBuild.yml)

---

## タグの使い方

このリポジトリでは、学習段階ごとにタグを付けて管理しています。

### タグの付け方 (Create Tag)

GitHub Actions の **[Create Tag](https://github.com/kitajima-hayato/MyEngine/actions/workflows/CreateTag.yml)** ワークフローを使用します。

1. Actions タブ → **Create Tag** → **Run workflow** をクリック
2. 以下の入力を行い、**Run workflow** を実行します

| 入力項目 | 説明 | 例 |
|---|---|---|
| `tag_name` | 付けるタグの名前 | `StatePattern` |
| `tag_message` | タグの説明（省略可） | `ステートパターンの実装` |
| `force_update` | 既存タグを上書きするか | `false` |

> **ローカルでタグを付ける場合**
> ```bash
> git tag -a StatePattern -m "ステートパターンの実装"
> git push origin StatePattern
> ```

---

### タグから最新の状態にする方法 (Sync From Tag)

GitHub Actions の **[Sync From Tag](https://github.com/kitajima-hayato/MyEngine/actions/workflows/SyncFromTag.yml)** ワークフローを使用します。

指定したタグのコミットを `tag/<タグ名>` ブランチとして作成・更新します。

1. Actions タブ → **Sync From Tag** → **Run workflow** をクリック
2. `tag_name` に同期したいタグ名を入力して **Run workflow** を実行します

実行後、`tag/<タグ名>` ブランチが作成されるので、ローカルでチェックアウトできます：

```bash
git fetch origin
git checkout tag/StatePattern
```

> **ローカルでタグの状態を確認する場合**
> ```bash
> # タグ一覧を表示
> git tag -l
>
> # タグの状態にチェックアウト
> git checkout StatePattern
>
> # 最新の master に戻る
> git checkout master
> git pull origin master
> ```

---

### 利用可能なタグ一覧

| タグ名 | 内容 |
|---|---|
| `DataDriven` | データ駆動設計 |
| `ExternalFile` | 外部ファイルの利用 |
| `MagicNumbers` | マジックナンバーの排除 |
| `NewDelete` | new/delete の適切な管理 |
| `DesignPattern` | デザインパターンの導入 |
| `Encapsulate` | カプセル化 |
| `Polymorphism` | ポリモーフィズム |
| `StatePattern` | ステートパターン |

