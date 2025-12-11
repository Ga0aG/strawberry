import requests
import json
import pandas as pd
import yaml
import shutil
import hashlib
import time
import os
import base64
import sys
import urllib
from typing import Dict, Any
from IPython import embed
from datetime import datetime, timedelta


class FeishuClient:
    def __init__(self, app_id, app_secret, sheet_token, sheet_id, read_range="A1:E"):
        self.app_id = app_id
        self.app_secret = app_secret
        self.sheet_token = sheet_token
        self.sheet_id = sheet_id
        self.read_range = read_range
        self.tenant_access_token = self.get_tenant_access_token()
        # self.user_token = self.get_user_access_token()
        # 如果没有提供 sheet_id，自动获取第一个 sheet
        if not self.sheet_id:
            self.sheet_id = self.get_first_sheet_id()
        self.last_hash = None
        self.base_dir = os.path.dirname(os.path.abspath(__file__))

    def get_tenant_access_token(self):
        """获取飞书租户访问令牌"""
        url = "https://open.feishu.cn/open-apis/auth/v3/tenant_access_token/internal/"
        headers = {"Content-Type": "application/json"}
        data = {"app_id": self.app_id, "app_secret": self.app_secret}
        response = requests.post(url, headers=headers, json=data)
        print(response.json())
        # tenant_access_token 的最大有效期是 2 小时。
        return response.json().get("tenant_access_token")

    def get_user_access_token(self, authorization_code: str):
        url = "https://open.feishu.cn/open-apis/authen/v2/oauth/token"
        payload = {
            "grant_type": "authorization_code",
            "client_id": self.app_id,
            "client_secret": self.app_secret,
            "code": authorization_code,
        }

        headers = {"Content-Type": "application/json; charset=utf-8"}

        try:
            print(f"POST: {url}")
            print(f"Request payload: {json.dumps(payload, ensure_ascii=False)}")

            response = requests.post(url, json=payload, headers=headers)
            response.raise_for_status()

            result = response.json()
            print(f"Response: {json.dumps(result, ensure_ascii=False, indent=2)}")

            if result.get("code", 0) != 0:
                error_msg = f"failed to get user_access_token: {result.get('msg', 'unknown error')}"
                print(f"ERROR: {error_msg}", file=sys.stderr)
                return {}, Exception(error_msg)

            return result, None

        except Exception as e:
            error_msg = f"Error getting user_access_token: {e}"
            if hasattr(e, "response") and e.response is not None:
                error_msg += f" Response: {e.response.text}"
            print(f"ERROR: {error_msg}", file=sys.stderr)
            return {}, e

    def get_first_sheet_id(self):
        """获取表格中第一个工作表的 ID"""
        url = f"https://open.feishu.cn/open-apis/sheets/v2/spreadsheets/{self.sheet_token}/metainfo"
        headers = {"Authorization": f"Bearer {self.tenant_access_token}"}
        response = requests.get(url, headers=headers)
        result = response.json()
        print(f"Sheet metadata: {result}")
        if result.get("code") == 0:
            sheets = result.get("data", {}).get("sheets", [])
            if sheets:
                sheet_id = sheets[0].get("sheetId")
                print(f"Auto-detected sheet_id: {sheet_id}")
                return sheet_id
        raise Exception(f"Failed to get sheet_id: {result}")

    def get_sheet_data(self):
        """获取飞书表格数据"""
        url = f"https://open.feishu.cn/open-apis/sheets/v2/spreadsheets/{self.sheet_token}/values/{self.sheet_id}!{self.read_range}?valueRenderOption=ToString&dateTimeRenderOption=FormattedString"
        print(f"url: {url}")
        headers = {"Authorization": f"Bearer {self.tenant_access_token}"}
        response = requests.get(url, headers=headers)
        return response.json()

    def append_data(self, data):
        """
        在表格末尾追加数据
        data: 二维列表，例如 [['分类1', '2024-01-01', '问题描述', '1小时', '机器人编号', '图片']]
        """
        url = f"https://open.feishu.cn/open-apis/sheets/v2/spreadsheets/{self.sheet_token}/values_append"
        headers = {
            "Authorization": f"Bearer {self.tenant_access_token}",
            "Content-Type": "application/json",
        }
        payload = {"valueRange": {"range": f"{self.sheet_id}!A:E", "values": data}}
        response = requests.post(url, headers=headers, json=payload)
        result = response.json()
        print(f"Append result: {result}")
        return result

    def delete_old_data(self, days_ago=1):
        """
        删除指定天数之前的数据
        days_ago: 删除多少天之前的数据，默认1天（删除昨天及更早的数据）
        """
        # 获取当前数据
        current_data = self.get_sheet_data()
        if current_data.get("code") != 0:
            raise Exception(f"Failed to get data: {current_data}")

        values = current_data.get("data", {}).get("valueRange", {}).get("values", [])

        if len(values) <= 1:
            print("No data to delete (only header exists)")
            return {"code": 0, "msg": "success"}

        # 计算截止日期
        cutoff_date = datetime.now() - timedelta(days=days_ago)
        cutoff_date_str = cutoff_date.strftime("%Y-%m-%d %H:%M")

        # 找出需要保留的数据（表头 + 新数据）
        header = values[0]
        rows_to_keep = [header]

        endIndex = 1
        for row in values[1:]:
            if len(row) >= 2:  # 确保有时间列
                row_date = row[1]  # '问题时间' 列
                try:
                    # 尝试解析日期（支持多种格式）
                    if " " in row_date:
                        row_date = row_date.split(" ")[0]  # 取日期部分

                    # 比较日期字符串
                    if row_date > cutoff_date_str:
                        rows_to_keep.append(row)
                        print(f"{row_date} > {cutoff_date_str}")
                    else:
                        print(f"{row_date} <= {cutoff_date_str}")
                        endIndex += 1
                except Exception as e:
                    print(f"Error parsing date '{row_date}': {e}")
                    # 日期解析失败，保留该行
                    rows_to_keep.append(row)

        deleted_count = len(values) - len(rows_to_keep)
        print(f"Found {deleted_count} rows to delete (older than {cutoff_date_str})")

        if deleted_count == 0:
            return {"code": 0, "msg": "No old data to delete"}

        url = f"https://open.feishu.cn/open-apis/sheets/v2/spreadsheets/{self.sheet_token}/dimension_range"
        headers = {
            "Authorization": f"Bearer {self.tenant_access_token}",
            "Content-Type": "application/json",
        }

        # 先清空
        payload = {
            "dimension": {
                "sheetId": self.sheet_id,
                "majorDimension": "ROWS",
                "startIndex": 2,
                "endIndex": endIndex,
            }
        }
        if endIndex > 2:
            print(f"Deleting rows from index 2 to {endIndex}")
            response = requests.delete(url, headers=headers, json=payload)
            embed()

        # 重新写入保留的数据
        # if len(rows_to_keep) > 0:
        #     write_payload = {
        #         "valueRanges": [
        #             {
        #                 "range": f"{self.sheet_id}!A1:E{len(rows_to_keep)}",
        #                 "values": rows_to_keep
        #             }
        #         ]
        #     }
        #     response = requests.put(url, headers=headers, json=write_payload)
        #     result = response.json()
        #     print(f"Delete old data result: {result}, deleted {deleted_count} rows")
        #     return result

        # return {"code": 0, "msg": f"Deleted {deleted_count} rows"}

    def clear_data(self, keep_header=True):
        """
        清空表格数据
        keep_header: True 保留第一行表头，False 清空所有数据
        """
        # 先获取当前数据行数
        current_data = self.get_sheet_data()
        if current_data.get("code") != 0:
            raise Exception(f"Failed to get data: {current_data}")

        values = current_data.get("data", {}).get("values", [])
        total_rows = len(values)

        if total_rows <= 1 and keep_header:
            print("No data to clear (only header exists)")
            return {"code": 0, "msg": "success"}

        # 确定要清空的范围
        if keep_header:
            clear_range = f"{self.sheet_id}!A2:E{total_rows}"
        else:
            clear_range = f"{self.sheet_id}!A1:E{total_rows}"

        # 清空数据
        url = f"https://open.feishu.cn/open-apis/sheets/v2/spreadsheets/{self.sheet_token}/values_batch_update"
        headers = {
            "Authorization": f"Bearer {self.tenant_access_token}",
            "Content-Type": "application/json",
        }
        payload = {"valueRanges": [{"range": clear_range, "values": [[]]}]}
        response = requests.put(url, headers=headers, json=payload)
        result = response.json()
        print(f"Clear result: {result}")
        return result

    def uploadImage(self):
        with open("test.png", "rb") as f:
            url = f"https://open.feishu.cn/open-apis/sheets/v2/spreadsheets/{self.sheet_token}/values_image"
            fb = f.read()
            misssing_padding = 4 - len(fb) % 4
            if misssing_padding:
                fb += b"=" * misssing_padding
            fb = base64.b64encode(fb).decode("utf-8")
            data = {
                "range": f"{self.sheet_id}!F2:F2",
                "image": fb,
                "name": "a.png",
            }
            headers = {
                "Authorization": f"Bearer {self.tenant_access_token}",
                "Content-Type": "application/json",
            }
            response = requests.post(url, data=json.dumps(data), headers=headers)
            print(response.json())

    def sendEmail(
        self,
        user_mailbox,
        to_addresses,
        subject,
        body_html=None,
        body_plain_text=None,
        cc=None,
        bcc=None,
        attachments=None,
        head_from_name=None,
    ):
        """
        发送邮件
        user_mailbox: 发件人邮箱地址，例如 "user@xxx.xx" 或 "me"
        to_addresses: 收件人列表，例如 [{"mail_address": "user@xxx.xx", "name": "Mike"}]
        subject: 邮件标题
        body_html: HTML格式邮件正文
        body_plain_text: 纯文本格式邮件正文
        cc: 抄送列表（可选）
        bcc: 密送列表（可选）
        attachments: 附件列表（可选），例如 [{"body": "base64编码内容", "filename": "file.txt", "is_inline": False}]
        head_from_name: 发件人显示名称（可选）
        """
        import urllib.parse

        encoded_mailbox = urllib.parse.quote(user_mailbox, safe="")
        url = f"https://open.feishu.cn/open-apis/mail/v1/user_mailboxes/{encoded_mailbox}/messages/send"

        headers = {
            "Authorization": f"Bearer {self.tenant_access_token}",
            "Content-Type": "application/json",
        }

        payload = {"to": to_addresses, "subject": subject}

        if body_html:
            payload["body_html"] = body_html
        if body_plain_text:
            payload["body_plain_text"] = body_plain_text
        if cc:
            payload["cc"] = cc
        if bcc:
            payload["bcc"] = bcc
        if attachments:
            payload["attachments"] = attachments
        if head_from_name:
            payload["head_from"] = {"name": head_from_name}

        response = requests.post(url, headers=headers, json=payload)
        result = response.json()
        print(f"Send email result: {result}")
        return result

    def add_record(self, app_token, table_id, record_data):
        """
        向飞书表格应用添加记录
        app_token: 应用访问令牌
        table_id: 表格ID
        record_data: 记录数据，字典格式，例如 {"字段1": "值1", "字段2": "值2"}
        """
        url = f"https://open.feishu.cn/open-apis/bitable/v1/apps/{app_token}/tables/{table_id}/records?user_id_type=user_id"
        headers = {
            "Authorization": f"Bearer {self.tenant_access_token}",
            "Content-Type": "application/json",
        }
        payload = {"fields": record_data}
        response = requests.post(url, headers=headers, json=payload)
        result = response.json()
        print(f"Add record result: {result}")
        return result

    def clear_records(self, app_token, table_id, page_size=100):
        """
        清空飞书表格应用中的所有记录
        app_token: 应用访问令牌
        table_id: 表格ID
        """
        # 获取所有记录ID
        url = f"https://open.feishu.cn/open-apis/bitable/v1/apps/{app_token}/tables/{table_id}/records?user_id_type=user_id&page_size={page_size}"
        headers = {
            "Authorization": f"Bearer {self.tenant_access_token}",
            "Content-Type": "application/json",
        }
        payload = {
            "field_names": ["Number"],
            "sort": [{"field_name": "Date Created", "desc": False}],
        }
        response = requests.get(url, headers=headers, json=payload)
        result = response.json()
        if result.get("code") != 0:
            raise Exception(f"Failed to get records: {result}")
        record_size = result.get("data", {}).get("total", 0)
        if record_size > 2:
            record_ids = [
                record["record_id"]
                for record in result.get("data", {}).get("items", [])[:-2]
            ]

            # 批量删除记录
            for record_id in record_ids:
                del_url = f"https://open.feishu.cn/open-apis/bitable/v1/apps/{app_token}/tables/{table_id}/records/{record_id}"
                del_response = requests.delete(del_url, headers=headers)
                del_result = del_response.json()
                print(f"Delete record {record_id} result: {del_result}")

    def get_wiki_node_info(self, node_token: str) -> Dict[str, Any]:
        """获取知识空间节点信息

        Args:
            tenant_access_token: 租户访问令牌
            node_token: 节点令牌

        Returns:
            Dict[str, Any]: 节点信息对象
        """
        # URL encode the node_token
        encoded_token = urllib.parse.quote(node_token)
        url = f"https://open.feishu.cn/open-apis/wiki/v2/spaces/get_node?token={encoded_token}"

        headers = {
            "Authorization": f"Bearer {self.tenant_access_token}",
            "Content-Type": "application/json; charset=utf-8",
        }

        try:
            print(f"GET: {url}")
            response = requests.get(url, headers=headers)
            response.raise_for_status()

            result = response.json()
            print(f"Response: {json.dumps(result)}")

            if result.get("code", 0) != 0:
                print(f"ERROR: 获取知识空间节点信息失败 {result}", file=sys.stderr)
                raise Exception(
                    f"failed to get wiki node info: {result.get('msg', 'unknown error')}"
                )

            if not result.get("data") or not result["data"].get("node"):
                raise Exception("未获取到节点信息")

            node_info = result["data"]["node"]
            print(
                "节点信息获取成功:",
                {
                    "node_token": node_info.get("node_token"),
                    "obj_type": node_info.get("obj_type"),
                    "obj_token": node_info.get("obj_token"),
                    "title": node_info.get("title"),
                },
            )
            return node_info

        except Exception as e:
            print(f"ERROR: Error getting wiki node info: {e}", file=sys.stderr)
            raise

    def get_parent_node_for_bitable_upload(self, wiki_node_token: str) -> str:
        """获取用于上传素材到Wiki多维表格的parent_node

        Args:
            wiki_node_token: 知识库节点token
            tenant_access_token: 租户访问令牌

        Returns:
            str: 多维表格的app_token，即parent_node值
        """

        # 获取知识空间节点信息
        node_info = self.get_wiki_node_info(wiki_node_token)

        # 检查是否为多维表格类型
        obj_type = node_info.get("obj_type")
        if obj_type != "bitable":
            print(f"WARNING: 节点类型不是多维表格(bitable)，当前类型: {obj_type}")

        # 获取多维表格的app_token作为parent_node
        parent_node = node_info.get("obj_token")
        if not parent_node:
            raise Exception("未能获取到多维表格的obj_token")

        print(f"成功获取parent_node: {parent_node}")
        return parent_node, node_info.get("obj_token")

    def upload_media(
        self, file_path, file_name, parent_node, parent_type="bitable_image", size=None
    ):
        """
        上传素材到飞书drive
        file_path: 本地文件路径
        file_name: 文件名
        parent_type: 父类型，例如 'bitable_image'
        parent_node: 父节点token
        size: 文件大小（可选，如果不提供则自动计算）
        """
        if size is None:
            size = os.path.getsize(file_path)

        url = "https://open.feishu.cn/open-apis/drive/v1/medias/upload_all"
        headers = {
            "Authorization": f"Bearer {self.tenant_access_token}",
            # "Content-Type": "multipart/form-data"
        }

        with open(file_path, "rb") as f:
            files = {
                "file": (file_name, f, "application/octet-stream"),
            }
            data = {
                "file_name": file_name,
                "parent_type": parent_type,
                "parent_node": parent_node,
                "size": str(size),
            }
            response = requests.post(url, headers=headers, files=files, data=data)
            result = response.json()
            # print(f"Upload media result: {result}")
            # return result
            if result.get("code") != 0:
                raise Exception(f"Failed to upload media: {result}")
            return result.get("data", {}).get("file_token", "")

    def add_bitable_record(self, app_token, table_id, fields):
        """
        向飞书多维表格添加记录
        app_token: 应用访问令牌
        table_id: 表格ID
        fields: 记录字段字典，例如 {"优先级": "P2", "详细描述": "测试", "附件": [{"file_token": "token"}]}
        """
        url = f"https://open.feishu.cn/open-apis/bitable/v1/apps/{app_token}/tables/{table_id}/records?user_id_type=user_id"
        headers = {
            "Authorization": f"Bearer {self.tenant_access_token}",
            "Content-Type": "application/json",
        }
        payload = {"fields": fields}
        response = requests.post(url, headers=headers, json=payload)
        result = response.json()
        print(f"Add bitable record result: {result}")
        return result

    def get_bitable_records(self, app_token, table_id, page_size=100):
        """
        获取飞书多维表格的所有记录
        app_token: 应用访问令牌
        table_id: 表格ID
        page_size: 每页记录数，默认100
        """
        url = f"https://open.feishu.cn/open-apis/bitable/v1/apps/{app_token}/tables/{table_id}/records/search?page_size={page_size}&user_id_type=user_id"
        headers = {
            "Authorization": f"Bearer {self.tenant_access_token}",
            "Content-Type": "application/json",
        }
        payload = {}
        response = requests.post(url, headers=headers, json=payload)
        result = response.json()
        print(f"Get bitable records result: {result}")
        """
        {
            {
                'code': 0,
                'data': {
                    'has_more': False,
                    'items': [
                        {
                            'fields': {
                                '优先级': 'P2',
                                '详细描述': [{'text': '测试', 'type': 'text'}],
                                '问题模块': '测试问题',
                                '附件': [
                                    {
                                        'file_token': 'NSCtb21niojymXxwTcFcMws6nzf',
                                        'name': 'Screenshot from 2025-12-03 10-41-04.png',
                                        'size': 9921,
                                        'tmp_url': 'https://open.feishu.cn/open-apis/drive/v1/medias/batch_get_tmp_download_url?file_tokens=NSCtb21niojymXxwTcFcMws6nzf',
                                        'type': 'image/png',
                                        'url': 'https://open.feishu.cn/open-apis/drive/v1/medias/NSCtb21niojymXxwTcFcMws6nzf/download'
                                    }
                                ]
                            },
                            'record_id': 'recv4XagKavXoi'
                        }
                    ],
                    'total': 24
                },
                'msg': 'success'
            }
        }
        page_token分页标记，当 has_more 为 true 时，会同时返回新的 page_token，否则不返回 page_token
        """
        return result

    def delete_bitable_record(self, app_token, table_id, record_id):
        """
        删除飞书多维表格中的记录
        app_token: 应用访问令牌
        table_id: 表格ID
        record_id: 记录ID
        """
        url = f"https://open.feishu.cn/open-apis/bitable/v1/apps/{app_token}/tables/{table_id}/records/{record_id}"
        headers = {
            "Authorization": f"Bearer {self.tenant_access_token}",
            "Content-Type": "application/json",
        }
        response = requests.delete(url, headers=headers)
        result = response.json()
        print(f"Delete bitable record result: {result}")
        return result

"""
APP_ID和APP_SECRET是在开发者平台创建应用建立的
"""

if __name__ == "__main__":
    APP_ID = "cli_a9905c8ff6bd9013"
    APP_SECRET = "ZGIS9hqU1f3yL4R7axaFglcDJn1uKmZg"
    SHEET_TOKEN = "PJ30sPI04huhEptJLpIcC7zUnkf"
    SHEET_ID = None
    client = FeishuClient(APP_ID, APP_SECRET, SHEET_TOKEN, SHEET_ID)

    # 更新普通表格
    # 示例：追加数据
    new_data = [
        ["网络问题", "2025-11-15 10:30", "服务器无法连接", "2小时", 1],
        ["性能问题", "2025-11-16 14:20", "页面加载缓慢", "30分钟", 1],
        ["网络问题", "2025-11-16 10:30", "服务器无法连接", "2小时", 1],
        ["性能问题", "2025-11-16 14:20", "页面加载缓慢", "30分钟", 1],
        ["网络问题", "2025-11-17 10:30", "服务器无法连接", "2小时", 1],
        ["性能问题", "2025-11-17 14:20", "页面加载缓慢", "30分钟", 1],
    ]
    # client.append_data(new_data)

    # # 示例：删除昨天及更早的数据
    # client.delete_old_data(days_ago=2)

    # 示例：删除7天前的数据
    # client.delete_old_data(days_ago=7)

    # 示例：清空数据（保留表头）
    # client.clear_data(keep_header=True)
    # client.uploadImage()

    # 示例： 更新wiki多维表格
    # https://g97aoekjur.feishu.cn/wiki/S827wUS2giJDV8kV5AFcVtYlnOf?table=tbl9UNEPaUcjmYaJ&view=vewF22LrI5
    wike_node_token = "S827wUS2giJDV8kV5AFcVtYlnOf"
    # I. 获得APP_TOKEN
    parent_node, app_token = client.get_parent_node_for_bitable_upload(wike_node_token)
    print(f"parent_node: {parent_node}, app_token: {app_token}")
    table_id = "tbl9UNEPaUcjmYaJ"
    # II. 上传附件，得到附件的file_token
    file_token = client.upload_media("/home/zhuojun/Pictures/Screenshots/Screenshot from 2025-12-03 10-41-04.png", "Screenshot from 2025-12-03 10-41-04.png", app_token)
    # file_token = "NSCtb21niojymXxwTcFcMws6nzf"
    # III. 添加记录
    client.add_bitable_record(
        app_token,
        table_id,
        {"优先级": "P2", "问题模块": "测试问题", "详细描述": "测试", "附件": [{"file_token": file_token}]},
    )
    # IV. 获取记录的id, 后续根据记录id来删除记录
    client.get_bitable_records(app_token, table_id)
    # V. 删除记录
    # client.delete_bitable_record(
    #     app_token, table_id, "recv4XagKavXoi"
    # )

    # 示例：发送邮件-不可行，还要授权。
    # client.sendEmail(
    #     user_mailbox="zhuojunlai@astribot.com",
    #     to_addresses=[{"mail_address": "zhuojunlai@astribot.com", "name": "zhuojunlai"}],
    #     subject="测试邮件",
    #     body_plain_text="这是一封测试邮件的正文。",
    #     )

    # print(client.get_sheet_data())

    # 示例：更新多维表格
    # app_token = "ZLqwb05I6ahB5ksYoS3cgo05nad"
    # table_id = "tblyZpMXD3ZFSIJ5"
    # client.add_record(
    #     app_token,
    #     table_id,
    #     {
    #         "email address": "zhuojunlai@astribot.com",
    #         "subject": "hello world",
    #         "content": "Nice to meet you!",
    #     },
    # )
    # client.clear_records(app_token, table_id)
